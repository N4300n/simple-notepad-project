#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include "spell_checker.h"

class highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public :
    highlighter(QTextDocument* parent, const spell_checker& checker);

protected:
    void highlightBlock(const QString& text) override;
private:
    const spell_checker& checker;
    QRegularExpression word_regex;
};

#endif