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
    ui->colour->setColour(newPen.color());
    ui->cosmeticBox->setChecked(newPen.isCosmetic());

    QString lineStyleStr;
    if (newPen.style() == Qt::SolidLine)
        ui->lineStyle->setCurrentIndex(0);
    else if (newPen.style() == Qt::DashLine)
        ui->lineStyle->setCurrentIndex(1);
    else if (newPen.style() == Qt::DotLine)
        ui->lineStyle->setCurrentIndex(2);
    else if (newPen.style() == Qt::DashDotLine)
        ui->lineStyle->setCurrentIndex(3);
    else if (newPen.style() == Qt::DashDotDotLine)
        ui->lineStyle->setCurrentIndex(4);
    //else if (newPen.style() == Qt::CustomDashLine)
    //    ui->lineStyle->setCurrentIndex(0);
    else
    {
        assert(false);
        ui->lineStyle->setCurrentIndex(-1);
    }

    ui->lineWidth->setValue(newPen.widthF());
}

QPen PenDialog::getPen() const
{
    QPen pen;
    pen.setColor(ui->colour->colour());
    pen.setCosmetic(ui->cosmeticBox->isChecked());

    Qt::PenStyle style = Qt::SolidLine;
    if (ui->lineStyle->currentIndex() == 0)
        style = Qt::SolidLine;
    else if (ui->lineStyle->currentIndex() == 1)
        style = Qt::DashLine;
    else if (ui->lineStyle->currentIndex() == 2)
        style = Qt::DotLine;
    else if (ui->lineStyle->currentIndex() == 3)
        style = Qt::DashDotLine;
    else if (ui->lineStyle->currentIndex() == 4)
        style = Qt::DashDotDotLine;
    else
    {
        assert(false);
    }

    pen.setStyle(style);

    pen.setWidth(static_cast<int>(ui->lineWidth->value()));

    return pen;
}
