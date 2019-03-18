#include "src/ui/widgets/ColourButton.h"
#include "qcolordialog.h"

ColourButton::ColourButton(QWidget* parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    setFlat(true);
    setColour(Qt::white);

    connect(this, &QPushButton::clicked, this, &ColourButton::onClick);
}

void ColourButton::setColour(const QColor& newColour)
{
    if (_colour == newColour) return;

    _colour = newColour;
    setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4)").arg(_colour.red()).arg(_colour.green()).arg(_colour.blue()).arg(_colour.alpha()));
    setText(QString("R: %1, G: %2, B: %3, A: %4")
        .arg(_colour.red(), 3, 10, QChar('0'))
        .arg(_colour.green(), 3, 10, QChar('0'))
        .arg(_colour.blue(), 3, 10, QChar('0'))
        .arg(_colour.alpha(), 3, 10, QChar('0')));

    emit colourChanged(_colour);
}

void ColourButton::onClick()
{
    QColor newColour = QColorDialog::getColor(_colour, this, "", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    if (newColour.isValid()) _colour = newColour;
}
