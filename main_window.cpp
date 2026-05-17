#include "main_window.h"
#include "notepad_exception.h"

#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSize>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QToolBar>

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

QTextDocument::FindFlags make_find_flags(const Ui::find_replace_dialog& ui)
{
    QTextDocument::FindFlags flags;
    if (ui.case_sensitive_check->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    return flags;
}

int count_status_words(const QString& text)
{
    return text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
}

int count_status_lines(const QString& text)
{
    if (text.isEmpty()) {
        return 1;
    }
    return text.count('\n') + 1;
}

void write_text_file(const QString& path, const QString& contents)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw file_write_exception(path.toStdString());
    }

    QTextStream out(&file);
    out << contents;
    if (out.status() != QTextStream::Ok) {
        throw file_write_exception(path.toStdString());
    }
}

}

main_window::main_window()
{
    setWindowTitle("Notepad");
    resize(800, 600);

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    transforms.push_back(std::make_unique<uppercase_transform>());
    transforms.push_back(std::make_unique<lowercase_transform>());
    transforms.push_back(std::make_unique<capitalize_transform>());
    transforms.push_back(std::make_unique<sentence_case_transform>());
    transforms.push_back(std::make_unique<swap_case_transform>());

    setup_file_menu();
    setup_edit_menu();
    setup_format_menu();
    setup_format_toolbar();
    setup_search_menu();
    setup_tools_menu();

    connect(editor, &QTextEdit::currentCharFormatChanged, this, [this](const QTextCharFormat&) {
        update_format_buttons();
    });
    connect(editor, &QTextEdit::textChanged, this, [this] {
        update_status_bar();
    });

    update_format_buttons();
    update_status_bar();
}

main_window::~main_window() = default;

void main_window::setup_file_menu()
{
    auto* file_menu = menuBar()->addMenu("File");

    const auto* action_new = file_menu->addAction("New");
    connect(action_new, &QAction::triggered, this, [this] {
        editor->clear();
        current_file.clear();
        update_title();
    });

    file_menu->addSeparator();

    const auto* action_open = file_menu->addAction("Open...");
    connect(action_open, &QAction::triggered, this, [this] {
        open_file();
    });

    const auto* action_save = file_menu->addAction("Save");
    connect(action_save, &QAction::triggered, this, [this] {
        save_file();
    });

    const auto* action_save_as = file_menu->addAction("Save As...");
    connect(action_save_as, &QAction::triggered, this, [this] {
        save_file_as();
    });

    file_menu->addSeparator();

    const auto* action_exit = file_menu->addAction("Exit");
    connect(action_exit, &QAction::triggered, this, [] {
        QApplication::quit();
    });
}

void main_window::setup_edit_menu()
{
    auto* edit_menu = menuBar()->addMenu("Edit");

    auto* action_undo = edit_menu->addAction("Undo");
    action_undo->setShortcut(QKeySequence::Undo);
    connect(action_undo, &QAction::triggered, editor, &QTextEdit::undo);

    auto* action_redo = edit_menu->addAction("Redo");
    action_redo->setShortcut(QKeySequence::Redo);
    connect(action_redo, &QAction::triggered, editor, &QTextEdit::redo);

    edit_menu->addSeparator();

    auto* action_cut = edit_menu->addAction("Cut");
    action_cut->setShortcut(QKeySequence::Cut);
    connect(action_cut, &QAction::triggered, editor, &QTextEdit::cut);

    auto* action_copy = edit_menu->addAction("Copy");
    action_copy->setShortcut(QKeySequence::Copy);
    connect(action_copy, &QAction::triggered, editor, &QTextEdit::copy);

    auto* action_paste = edit_menu->addAction("Paste");
    action_paste->setShortcut(QKeySequence::Paste);
    connect(action_paste, &QAction::triggered, editor, &QTextEdit::paste);

    edit_menu->addSeparator();

    auto* action_select_all = edit_menu->addAction("Select All");
    action_select_all->setShortcut(QKeySequence::SelectAll);
    connect(action_select_all, &QAction::triggered, editor, &QTextEdit::selectAll);
}

void main_window::setup_format_menu()
{
    auto* format_menu = menuBar()->addMenu("Format");
    auto* text_case_menu = format_menu->addMenu("Text Case");

    for (const auto& transform : transforms) {
        auto* action = text_case_menu->addAction(QString::fromStdString(transform->name()));
        connect(action, &QAction::triggered, this, [this, transform_ptr = transform.get()] {
            apply_transform(*transform_ptr);
        });
    }
}

void main_window::setup_format_toolbar()
{
    auto* toolbar = addToolBar("Format");
    toolbar->setIconSize(QSize(16, 16));

    bold_action = toolbar->addAction(QIcon("data/images/bold.svg"), "Bold");
    bold_action->setCheckable(true);
    bold_action->setShortcut(QKeySequence::Bold);
    connect(bold_action, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        editor->mergeCurrentCharFormat(format);
    });

    italic_action = toolbar->addAction(QIcon("data/images/italic.svg"), "Italic");
    italic_action->setCheckable(true);
    italic_action->setShortcut(QKeySequence::Italic);
    connect(italic_action, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontItalic(checked);
        editor->mergeCurrentCharFormat(format);
    });

    underline_action = toolbar->addAction(QIcon("data/images/underline.svg"), "Underline");
    underline_action->setCheckable(true);
    underline_action->setShortcut(QKeySequence::Underline);
    connect(underline_action, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontUnderline(checked);
        editor->mergeCurrentCharFormat(format);
    });
}

void main_window::setup_search_menu()
{
    auto* search_menu = menuBar()->addMenu("Search");

    auto* action_find_replace = search_menu->addAction("Find / Replace...");
    action_find_replace->setShortcut(QKeySequence::Find);
    connect(action_find_replace, &QAction::triggered, this, [this] {
        show_find_replace_dialog();
    });
}

void main_window::setup_tools_menu()
{
    auto* tools_menu = menuBar()->addMenu("Tools");

    const auto* action_word_freq = tools_menu->addAction("Word Frequency...");
    connect(action_word_freq, &QAction::triggered, this, [this] {
        show_word_frequency();
    });
}

void main_window::apply_transform(const text_transform& transform) const
{
    auto cursor = editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
    }

    QString text = cursor.selectedText();
    text.replace(QChar::ParagraphSeparator, '\n');
    text.replace(QChar::LineSeparator, '\n');

    const auto result = transform.apply(text.toStdString());

    cursor.beginEditBlock();
    cursor.insertText(QString::fromStdString(result));
    cursor.endEditBlock();
}

void main_window::open_file()
{
    const auto path = QFileDialog::getOpenFileName(this, "Open File");
    if (path.isEmpty()) {
        return;
    }

    try {
        QFile file(path);
        if (!file.exists()) {
            throw file_not_found_exception(path.toStdString());
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw file_read_exception(path.toStdString());
        }

        QTextStream in(&file);
        const auto contents = in.readAll();
        if (in.status() != QTextStream::Ok) {
            throw file_read_exception(path.toStdString());
        }

        editor->setPlainText(contents);
        current_file = path;
        update_title();
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
    }
}

void main_window::save_file()
{
    if (current_file.isEmpty()) {
        save_file_as();
        return;
    }
    try {
        write_text_file(current_file, editor->toPlainText());
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
    }
}

void main_window::save_file_as()
{
    const auto path = QFileDialog::getSaveFileName(this, "Save File As");
    if (path.isEmpty()) {
        return;
    }

    try {
        write_text_file(path, editor->toPlainText());
        current_file = path;
        update_title();
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
    }
}

void main_window::update_title()
{
    if (current_file.isEmpty()) {
        setWindowTitle("Notepad");
    } else {
        setWindowTitle("Notepad: " + current_file);
    }
}

void main_window::update_format_buttons()
{
    const auto current = editor->currentCharFormat();

    if (bold_action != nullptr) {
        bold_action->setChecked(current.fontWeight() == QFont::Bold);
    }
    if (italic_action != nullptr) {
        italic_action->setChecked(current.fontItalic());
    }
    if (underline_action != nullptr) {
        underline_action->setChecked(current.fontUnderline());
    }
}

void main_window::update_status_bar()
{
    const QString text = editor->toPlainText();
    statusBar()->showMessage(
        QString("Words: %1  Lines: %2").arg(count_status_words(text)).arg(count_status_lines(text)));
}

void main_window::show_find_replace_dialog()
{
    if (!find_replace_dlg) {
        find_replace_dlg = new QDialog(this);
        find_replace_ui = std::make_unique<Ui::find_replace_dialog>();
        find_replace_ui->setupUi(find_replace_dlg);

        connect(find_replace_ui->find_next_button, &QPushButton::clicked,
            this, [this] {
                find_next(find_replace_ui->find_input->text(), make_find_flags(*find_replace_ui));
            });
        connect(find_replace_ui->replace_button, &QPushButton::clicked,
            this, [this] {
                replace_current(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), make_find_flags(*find_replace_ui));
            });
        connect(find_replace_ui->replace_all_button, &QPushButton::clicked,
            this, [this] {
                replace_all(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), make_find_flags(*find_replace_ui));
            });
        connect(find_replace_ui->close_button, &QPushButton::clicked,
            find_replace_dlg, [this] { find_replace_dlg->hide(); });
    }

    find_replace_dlg->show();
    find_replace_dlg->raise();
    find_replace_dlg->activateWindow();
}

void main_window::find_next(const QString& term, const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }

    auto found = editor->document()->find(term, editor->textCursor(), flags);
    if (found.isNull()) {
        auto from_start = editor->textCursor();
        from_start.movePosition(QTextCursor::Start);
        found = editor->document()->find(term, from_start, flags);
    }
    if (!found.isNull()) {
        editor->setTextCursor(found);
    }
}

void main_window::replace_current(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (auto cursor = editor->textCursor(); cursor.hasSelection()) {
        cursor.insertText(replacement);
        editor->setTextCursor(cursor);
    }
    find_next(term, flags);
}

void main_window::replace_all(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto start_cursor = editor->textCursor();
    start_cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(start_cursor);

    while (editor->find(term, flags)) {
        auto cursor = editor->textCursor();
        cursor.insertText(replacement);
        editor->setTextCursor(cursor);
    }
}

void main_window::show_word_frequency()
{
    const auto text = editor->toPlainText().toLower().toStdString();

    std::map<std::string, int> freq;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        std::erase_if(word, [](const unsigned char c) {
            return !std::isalpha(c);
        });
        if (!word.empty()) {
            ++freq[word];
        }
    }

    std::vector<std::pair<std::string, int>> sorted_freq(freq.begin(), freq.end());
    std::sort(sorted_freq.begin(), sorted_freq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    auto* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    Ui::word_frequency_dialog ui;
    ui.setupUi(dialog);

    ui.frequency_table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui.frequency_table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ui.frequency_table->setRowCount(static_cast<int>(sorted_freq.size()));
    for (int i = 0; i < static_cast<int>(sorted_freq.size()); ++i) {
        const auto& [w, count] = sorted_freq[i];
        ui.frequency_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(w)));
        auto* count_item = new QTableWidgetItem(QString::number(count));
        count_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.frequency_table->setItem(i, 1, count_item);
    }
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    dialog->exec();
}