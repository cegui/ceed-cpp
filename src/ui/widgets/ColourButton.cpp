#include "src/ui/widgets/ColourButton.h"
#include "qcolordialog.h"
#include "qpainter.h"

ColourButton::ColourButton(QWidget* parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    //setFlat(true);
    setIconSize(QSize(24, 24));
    setColour(Qt::white);

    connect(this, &QPushButton::clicked, this, &ColourButton::onClick);
}

void ColourButton::setColour(const QColor& newColour)
{
    if (_colour == newColour) return;

    //??? to utils?
    QPixmap pixmap(24, 24);
    pixmap.fill(QColor(newColour));
    QPainter painter(&pixmap);
    painter.drawRect(0, 0, 23, 23);
    setIcon(QIcon(pixmap));

    _colour = newColour;
    setText(QString("R: %1, G: %2, B: %3, A: %4")
        .arg(_colour.red(), 3, 10, QChar('0'))
        .arg(_colour.green(), 3, 10, QChar('0'))
        .arg(_colour.blue(), 3, 10, QChar('0'))
        .arg(_colour.alpha(), 3, 10, QChar('0')));

    emit colourChanged(_colour);
}

void ColourButton::onClick()
{
    // We pass parent widget because we may use setStyleSheet and we don't want to propagate it to QColorDialog
    QColor newColour = QColorDialog::getColor(_colour, parentWidget(), "", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    if (newColour.isValid()) setColour(newColour);
}
