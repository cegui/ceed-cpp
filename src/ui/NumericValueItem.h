#ifndef NUMERICVALUEITEM_H
#define NUMERICVALUEITEM_H

#include "qgraphicsitem.h"

// Graphics scene item which represents a numeric value with optional editing capability.
// When not edited, value can be accompanied by some additional text information.

class QDoubleValidator;

class NumericValueItem : public QGraphicsTextItem
{
    Q_OBJECT

public:

    NumericValueItem(QGraphicsItem* parent = nullptr);
    virtual ~NumericValueItem() override;

    void setValue(qreal value);
    void setPrecision(int fracDigits);
    void setTextTemplate(const QString& tpl);

Q_SIGNALS:

    void valueChanged(qreal newValue);

protected:

    void updateText();

    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    QDoubleValidator* _inputValidator = nullptr;
    qreal _value = 0.0;
    int _precision = -1;
    QString _template = "%1";
    bool _isFocused = false;
    bool _ignoreNextMouseRelease = false;
};

#endif // NUMERICVALUEITEM_H
