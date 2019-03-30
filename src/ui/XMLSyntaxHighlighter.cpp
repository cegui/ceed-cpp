#include "src/ui/XMLSyntaxHighlighter.h"

XMLSyntaxHighlighter::XMLSyntaxHighlighter(QObject* parent)
    : QSyntaxHighlighter(parent)
{
    init();
}

XMLSyntaxHighlighter::XMLSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    init();
}

void XMLSyntaxHighlighter::init()
{
    // TODO: some fail colour highlighting :D please someone change the colours
    QTextCharFormat keywordFormat;
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::darkCyan);

    highlightingRules.emplace_back(QRegularExpression("\\b?xml\\b"), keywordFormat);
    highlightingRules.emplace_back(QRegularExpression("/>"), keywordFormat);
    highlightingRules.emplace_back(QRegularExpression(">"), keywordFormat);
    highlightingRules.emplace_back(QRegularExpression("</"), keywordFormat);
    highlightingRules.emplace_back(QRegularExpression("<"), keywordFormat);

    QTextCharFormat elementNameFormat;
    elementNameFormat.setFontWeight(QFont::Bold);
    elementNameFormat.setForeground(Qt::darkCyan);

    highlightingRules.emplace_back(QRegularExpression("\\b[A-Za-z0-9_]+(?=[\\s/>])"), elementNameFormat);

    QTextCharFormat attributeKeyFormat;
    attributeKeyFormat.setFontItalic(true);
    attributeKeyFormat.setForeground(Qt::blue);
    highlightingRules.emplace_back(QRegularExpression("\\b[A-Za-z0-9_]+(?=\\=)"), attributeKeyFormat);
}

void XMLSyntaxHighlighter::highlightBlock(const QString& text)
{
    for (auto&& pair : highlightingRules)
    {
        auto it = pair.first.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            if (match.hasMatch())
                setFormat(match.capturedStart(), match.capturedLength(), pair.second);
        }
    }
}
