#include "src/ui/NumericValueItem.h"
#include "qtextcursor.h"
#include "qevent.h"
#include "qgraphicssceneevent.h"
#include "qtextdocument.h"
#include "qlocale.h"
#include "qpainter.h"
#include "qcursor.h"

NumericValueItem::NumericValueItem(QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    //setFlags(ItemIgnoresTransformations); - requires deviceTransform for positioning
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setCursor(Qt::IBeamCursor);
}

void NumericValueItem::setValue(qreal value)
{
    _value = value;
    updateText();
}

void NumericValueItem::setPrecision(int fracDigits)
{
    _precision = fracDigits;
    updateText();
}

void NumericValueItem::setTextTemplate(const QString& tpl)
{
    _template = tpl.isEmpty() ? "%1" : tpl;
    updateText();
}

void NumericValueItem::setHorizontalAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    setTextCursor(cursor);
}

void NumericValueItem::onTextChanged()
{
    bool ok = false;
    textToValue(&ok);
    if (!ok)
    {
        disconnect(document(), &QTextDocument::contentsChange, this, &NumericValueItem::onTextChanged);
        setPlainText(_lastValidText);
        connect(document(), &QTextDocument::contentsChange, this, &NumericValueItem::onTextChanged);

        QTextCursor cursor = textCursor();
        cursor.setPosition(_lastValidCursorPos);
        setTextCursor(cursor);
    }
    else
    {
        _lastValidText = toPlainText();
        _lastValidCursorPos = textCursor().position();
    }
}

void NumericValueItem::updateText()
{
    const bool focused = hasFocus();
    const QString tpl = focused ? "%1" : _template;
    setPlainText(tpl.arg(_value, 0, 'f', focused ? -1 : _precision));
}

// qFuzzyCompare doesn't work with doubles when one of them may be 0.0
// See docs: https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare
static inline bool compareReal(qreal a, qreal b) { return std::abs(a - b) < static_cast<qreal>(0.0001); }

void NumericValueItem::acceptNewValue()
{
    bool ok = false;
    qreal newValue = textToValue(&ok);
    if (ok && !compareReal(_value, newValue))
    {
        _value = newValue;
        emit valueChanged(newValue);
    }
}

// The most user-friendly string to qreal conversion
qreal NumericValueItem::textToValue(bool* ok) const
{
    QString text = toPlainText();

    // Empty string is OK, we want to be able to erase all text during an editing.
    // Value won't be changed, so accepting empty string cancels editing.
    if (text.isEmpty())
    {
        if (ok) *ok = true;
        return _value;
    }

    // Enforce single line
    if (text.indexOf('\n') >= 0 || text.indexOf('\r') >= 0)
    {
        if (ok) *ok = false;
        return 0.0;
    }

    QLocale locale;
    const QChar decimalPt = locale.decimalPoint();

    text.replace('.', decimalPt);
    text.replace(',', decimalPt);

    return locale.toDouble(text, ok);
}

// It seems that QGraphicsTextItem contains at least 50% of all Qt bugs :( It is broken in all possible ways.
// FIXME: Qt 5.12.3 crashes in unmodified QGraphicsTextItem::paint when rendering to OpenGL in edit mode.
// FIXME: after closing and reopening layout editor text render as rectangles without alpha.
// FIXME: if render all texts to images, the first renders good and others as black rectangles.
// TODO: try OpenGL 3.2 Core renderer.
void NumericValueItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!painter) return;

    if (hasFocus())
    {
        QImage image(boundingRect().size().toSize(), QImage::Format_ARGB32);
        image.fill(QColor(255, 255, 255, 0));
        QPainter imagePainter(&image);
        QGraphicsTextItem::paint(&imagePainter, option, widget);
        imagePainter.end();

        painter->drawImage(boundingRect(), image, image.rect());
    }
    else
    {
        /*
        painter->save();
        painter->setBrush(defaultTextColor());
        painter->setFont(font());
        painter->setBackgroundMode(Qt::TransparentMode);
        painter->setBackground(Qt::transparent);
        painter->drawText(boundingRect(), toPlainText());
        painter->restore();
        */
        QGraphicsTextItem::paint(painter, option, widget);
    }
}

void NumericValueItem::focusInEvent(QFocusEvent* event)
{
    QGraphicsTextItem::focusInEvent(event);

    updateText();

    _lastValidText = toPlainText();
    _lastValidCursorPos = textCursor().position();

    _acceptValueOnFocusOut = true;

    // Select all text
    auto cursor = textCursor();
    cursor.select(QTextCursor::Document);
    setTextCursor(cursor);
    _ignoreNextMouseRelease = true;

    connect(document(), &QTextDocument::contentsChange, this, &NumericValueItem::onTextChanged);
}

void NumericValueItem::focusOutEvent(QFocusEvent* event)
{
    disconnect(document(), &QTextDocument::contentsChange, this, &NumericValueItem::onTextChanged);

    QGraphicsTextItem::focusOutEvent(event);

    if (_acceptValueOnFocusOut) acceptNewValue();

    updateText();
}

void NumericValueItem::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        {
            event->accept();
            _acceptValueOnFocusOut = (event->key() == Qt::Key_Enter);
            clearFocus();
            return;
        }
    }

    QGraphicsTextItem::keyPressEvent(event);
}

void NumericValueItem::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        {
            event->accept();
            return;
        }
    }

    QGraphicsTextItem::keyReleaseEvent(event);
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
