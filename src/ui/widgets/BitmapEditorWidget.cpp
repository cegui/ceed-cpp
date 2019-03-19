#include "src/ui/widgets/BitmapEditorWidget.h"
#include "ui_BitmapEditorWidget.h"

BitmapEditorWidget::BitmapEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BitmapEditorWidget)
{
    ui->setupUi(this);
}

BitmapEditorWidget::~BitmapEditorWidget()
{
    delete ui;
}

void BitmapEditorWidget::openFile(const QString& filePath)
{
    ui->preview->setPixmap(QPixmap(filePath));
}
