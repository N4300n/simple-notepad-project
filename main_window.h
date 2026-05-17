#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "text_transform.h"
#include "spell_checker.h"
#include "spell_checker_highlighter.h"

#include <QAction>
#include <QDialog>
#include <QMainWindow>
#include <QString>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextEdit>
#include <QLabel>

#include <memory>
#include <vector>

namespace Ui {
class find_replace_dialog;
}

class main_window : public QMainWindow {
public:
    main_window();
    ~main_window() override;

private:
    void setup_file_menu();
    void setup_edit_menu();
    void setup_format_menu();
    void setup_format_toolbar();
    void setup_search_menu();
    void setup_tools_menu();
    void setup_view_menu();

    void open_file();
    void save_file();
    void save_file_as();
    void update_title();

    void apply_transform(const text_transform& transform) const;
    void update_format_buttons();
    void update_status_bar();
    void update_cursor_position();

    void show_find_replace_dialog();
    void find_next(const QString& term, QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;
    void replace_current(const QString& term, const QString& replacement,
        QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;
    void replace_all(const QString& term, const QString& replacement,
        QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;

    void show_word_frequency();
    void show_spell_check();
    
    // Font and color functions
    void show_font_dialog();
    void show_text_color_dialog();
    
    // Zoom functions
    void zoom_in();
    void zoom_out();
    void reset_zoom();
    
    // Spell checker context menu
    void on_editor_context_menu_requested(const QPoint& pos);

    QTextEdit* editor { nullptr };
    QString current_file;
    std::vector<std::unique_ptr<text_transform>> transforms;

    QAction* bold_action { nullptr };
    QAction* italic_action { nullptr };
    QAction* underline_action { nullptr };
    QDialog* find_replace_dlg { nullptr };
    std::unique_ptr<Ui::find_replace_dialog> find_replace_ui;
    
    // Spell checker
    std::shared_ptr<spell_checker> spell_checker_obj;
    spell_checker_highlighter* spell_highlighter { nullptr };
    
    // Status bar labels
    QLabel* cursor_label { nullptr };
    QLabel* word_count_label { nullptr };
    
    // Zoom level
    int zoom_level { 100 };
};

#endif // MAIN_WINDOW_H
