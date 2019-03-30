#ifndef XMLSYNTAXHIGHLIGHTER_H
#define XMLSYNTAXHIGHLIGHTER_H

#include "qsyntaxhighlighter.h"
#include "qregularexpression.h"

class XMLSyntaxHighlighter : public QSyntaxHighlighter
{
public:

    XMLSyntaxHighlighter(QObject* parent = nullptr);
    XMLSyntaxHighlighter(QTextDocument* parent = nullptr);

protected:

    void init();

    virtual void highlightBlock(const QString& text) override;

    std::vector<std::pair<QRegularExpression, QTextCharFormat>> highlightingRules;
};

#endif // XMLSYNTAXHIGHLIGHTER_H
