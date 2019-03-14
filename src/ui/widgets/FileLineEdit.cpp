#include "src/ui/widgets/FileLineEdit.h"
#include "ui_FileLineEdit.h"
#include "qlineedit.h"
#include "qfiledialog.h"

FileLineEdit::FileLineEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileLineEdit),
    filter("Any file (*.*)")
{
    ui->setupUi(this);

    lineEdit = findChild<QLineEdit*>("lineEdit");
    getInitialDirectory = []() { return ""; };
}

FileLineEdit::~FileLineEdit()
{
    delete ui;
}

void FileLineEdit::setText(const QString& text)
{
    lineEdit->setText(text);
}

QString FileLineEdit::text() const
{
    return lineEdit->text();
}

void FileLineEdit::on_browseButton_pressed()
{
    QString path;
    switch (mode)
    {
        case Mode::NewFile:
        {
            path = QFileDialog::getSaveFileName(this, "Choose a path", getInitialDirectory(), filter);
            break;
        }
        case Mode::ExistingFile:
        {
            path = QFileDialog::getOpenFileName(this, "Choose a path", getInitialDirectory(), filter);
            break;
        }
        case Mode::ExistingDirectory:
        {
            path = QFileDialog::getExistingDirectory(this, "Choose a directory", getInitialDirectory());
            break;
        }
    }

    if (!path.isEmpty())
        lineEdit->setText(path);
}
