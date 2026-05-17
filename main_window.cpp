#include "main_window.h"
#include "notepad_exception.h"

#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontDialog>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
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
    : checker("data/words.txt")
{
    setWindowTitle("Notepad");
    resize(800, 600);

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editor, &QTextEdit::customContextMenuRequested, this, &main_window::show_spell_context_menu);

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
    setup_view_menu();
    setup_status_bar();

    highlighter = new spell_checker_highlighter(editor->document(), checker);

    connect(editor, &QTextEdit::currentCharFormatChanged, this, [this](const QTextCharFormat&) {
        update_format_buttons();
    });

    connect(editor, &QTextEdit::textChanged, this, [this] {
        update_status_bar();
    });

    connect(editor, &QTextEdit::cursorPositionChanged, this, &main_window::update_cursor_position);

    update_format_buttons();
    update_status_bar();
    update_cursor_position();
}

main_window::~main_window() = default;

void main_window::setup_file_menu()
{
    auto* file_menu = menuBar()->addMenu("File");

    const auto* action_new = file_menu->addAction("New");
    action_new->setShortcut(QKeySequence::New);
    connect(action_new, &QAction::triggered, this, [this] {
        editor->clear();
        current_file.clear();
        update_title();
    });

    file_menu->addSeparator();

    const auto* action_open = file_menu->addAction("Open...");
    action_open->setShortcut(QKeySequence::Open);
    connect(action_open, &QAction::triggered, this, [this] {
        open_file();
    });

    const auto* action_save = file_menu->addAction("Save");
    action_save->setShortcut(QKeySequence::Save);
    connect(action_save, &QAction::triggered, this, [this] {
        save_file();
    });

    const auto* action_save_as = file_menu->addAction("Save As...");
    action_save_as->setShortcut(QKeySequence::SaveAs);
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

    format_menu->addSeparator();

    auto* action_font = format_menu->addAction("Font...");
    connect(action_font, &QAction::triggered, this, &main_window::show_font_dialog);

    auto* action_color = format_menu->addAction("Text Color...");
    connect(action_color, &QAction::triggered, this, &main_window::show_color_dialog);
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

    toolbar->addSeparator();

    auto* zoom_in_action = toolbar->addAction("A+");
    connect(zoom_in_action, &QAction::triggered, this, &main_window::zoom_in);

    auto* zoom_out_action = toolbar->addAction("A-");
    connect(zoom_out_action, &QAction::triggered, this, &main_window::zoom_out);
}