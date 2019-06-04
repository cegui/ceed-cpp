#ifndef NUMERICVALUEITEM_H
#define NUMERICVALUEITEM_H

#include "qgraphicsitem.h"

// Graphics scene item which represents a numeric value with optional editing capability.
// When not edited, value can be accompanied by some additional text information.
// Enter is used to accept edited value, Esc to discard. Sometimes Enter breaks due to some
// internal Qt problem. Just click anywhere outside an item for it to lose focus, and value
// will be accepted.

class NumericValueItem : public QGraphicsTextItem
{
    Q_OBJECT

public:

    NumericValueItem(QGraphicsItem* parent = nullptr);

    void setValue(qreal value);
    void setPrecision(int fracDigits);
    void setTextTemplate(const QString& tpl);
    void setHorizontalAlignment(Qt::Alignment alignment);

Q_SIGNALS:

    void valueChanged(qreal newValue);

protected slots:

    void onTextChanged();

protected:

    void updateText();
    void acceptNewValue();
    qreal textToValue(bool* ok = nullptr) const;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    QString _lastValidText;
    int _lastValidCursorPos = 0;
    qreal _value = 0.0;
    int _precision = -1;
    QString _template = "%1";
    bool _ignoreNextMouseRelease = false;
    bool _acceptValueOnFocusOut = true;
};

#endif // NUMERICVALUEITEM_H
