#include "src/ui/FileSystemBrowser.h"
#include "ui_FileSystemBrowser.h"

FileSystemBrowser::FileSystemBrowser(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FileSystemBrowser)
{
    ui->setupUi(this);
}

FileSystemBrowser::~FileSystemBrowser()
{
    delete ui;
}
