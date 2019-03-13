#include "src/ui/widgets/FileLineEdit.h"
#include "ui_FileLineEdit.h"

FileLineEdit::FileLineEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileLineEdit)
{
    ui->setupUi(this);
}

FileLineEdit::~FileLineEdit()
{
    delete ui;
}
