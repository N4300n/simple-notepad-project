#include "highlighter.h"
#include <QTextCharFormat>

highlighter::highlighter(QTextDocument* parent, const spell_checker& checker)
    : QSyntaxHighlighter(parent), checker(checker) {
    word_regex = QRegularExpression("\\b[A-Za-z]+\\b");
}

void highlighter::highlightBlock(const QString& text) {
    QTextCharFormat error_format;
    error_format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    error_format.setUnderlineColor(Qt::red);

    QRegularExpressionMatchIterator i = word_regex.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        std::string word = match.captured(0).toLower().toStdString();
        
        if (!checker.is_correct(word)) {
            setFormat(match.capturedStart(), match.capturedLength(), error_format);
        }
    }
}