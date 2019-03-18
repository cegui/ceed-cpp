#include "src/ui/dialogs/PenDialog.h"
#include "ui_PenDialog.h"
#include "src/ui/widgets/ColourButton.h"

PenDialog::PenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PenDialog)
{
    ui->setupUi(this);
}

PenDialog::~PenDialog()
{
    delete ui;
}

void PenDialog::setPen(const QPen& newPen)
{
    auto colour = findChild<ColourButton*>("colour");
    auto lineStyle = findChild<QComboBox*>("lineStyle");
    auto lineWidth = findChild<QDoubleSpinBox*>("lineWidth");

    colour->setColour(newPen.color());


    QString lineStyleStr;
    if (newPen.style() == Qt::SolidLine)
        lineStyle->setCurrentIndex(0);
    else if (newPen.style() == Qt::DashLine)
        lineStyle->setCurrentIndex(1);
    else if (newPen.style() == Qt::DotLine)
        lineStyle->setCurrentIndex(2);
    else if (newPen.style() == Qt::DashDotLine)
        lineStyle->setCurrentIndex(3);
    else if (newPen.style() == Qt::DashDotDotLine)
        lineStyle->setCurrentIndex(4);
    //else if (newPen.style() == Qt::CustomDashLine)
    //    lineStyle->setCurrentIndex(0);
    else
    {
        assert(false);
        lineStyle->setCurrentIndex(-1);
    }

    lineWidth->setValue(newPen.widthF());
}

QPen PenDialog::getPen() const
{
    auto colour = findChild<ColourButton*>("colour");
    auto lineStyle = findChild<QComboBox*>("lineStyle");
    auto lineWidth = findChild<QDoubleSpinBox*>("lineWidth");

    QPen pen;
    pen.setColor(colour->colour());

    Qt::PenStyle style = Qt::SolidLine;
    if (lineStyle->currentIndex() == 0)
        style = Qt::SolidLine;
    else if (lineStyle->currentIndex() == 1)
        style = Qt::DashLine;
    else if (lineStyle->currentIndex() == 2)
        style = Qt::DotLine;
    else if (lineStyle->currentIndex() == 3)
        style = Qt::DashDotLine;
    else if (lineStyle->currentIndex() == 4)
        style = Qt::DashDotDotLine;
    else
    {
        assert(false);
    }

    pen.setStyle(style);

    pen.setWidth(lineWidth->value());

    return pen;
}
