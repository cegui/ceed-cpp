#include "src/ui/NumericValueItem.h"
#include "qtextcursor.h"
#include "qvalidator.h"
#include "qevent.h"
#include "qgraphicssceneevent.h"

NumericValueItem::NumericValueItem(QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);

    _inputValidator = new QDoubleValidator();
    _inputValidator->setDecimals(_precision < 0 ? 20 : _precision);
    _inputValidator->setNotation(QDoubleValidator::StandardNotation);
}

NumericValueItem::~NumericValueItem()
{
    delete _inputValidator;
}

void NumericValueItem::setValue(qreal value)
{
    _value = value;
    updateText();
}

void NumericValueItem::setPrecision(int fracDigits)
{
    _precision = fracDigits;
    _inputValidator->setDecimals(_precision < 0 ? 20 : _precision);
    updateText();
}

void NumericValueItem::setTextTemplate(const QString& tpl)
{
    _template = tpl.isEmpty() ? "%1" : tpl;
    updateText();
}

void NumericValueItem::updateText()
{
    const QString tpl = _isFocused ? "%1" : _template;
    setPlainText(tpl.arg(_value, 0, 'f', _isFocused ? -1 : _precision));
}

void NumericValueItem::focusInEvent(QFocusEvent* event)
{
    QGraphicsTextItem::focusInEvent(event);
    _isFocused = true;
    updateText();

    // Select all text
    auto cursor = textCursor();
    cursor.select(QTextCursor::Document);
    setTextCursor(cursor);
    _ignoreNextMouseRelease = true;
}

void NumericValueItem::focusOutEvent(QFocusEvent* event)
{
    _isFocused = false;
    updateText();
    QGraphicsTextItem::focusOutEvent(event);
}

// qFuzzyCompare doesn't work with doubles when one of them may be 0.0
// See docs: https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare
static inline bool compareReal(qreal a, qreal b) { return std::abs(a - b) < static_cast<qreal>(0.0001); }

void NumericValueItem::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            clearFocus();
            event->accept();
            return;
        }
        case Qt::Key_Enter:
        {
            bool ok = false;
            qreal newValue = toPlainText().toDouble(&ok);
            if (ok && !compareReal(_value, newValue))
            {
                _value = newValue;
                emit valueChanged(newValue);
            }

            clearFocus();
            event->accept();
            return;
        }
    }

    //_inputValidator->validate(), fixup()?

    QGraphicsTextItem::keyPressEvent(event);
}

void NumericValueItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (_ignoreNextMouseRelease)
    {
        _ignoreNextMouseRelease = false;
        event->accept();
        return;
    }

    QGraphicsTextItem::mouseReleaseEvent(event);
}
