#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include <QDockWidget>

namespace Ui {
class FileSystemBrowser;
}

class FileSystemBrowser : public QDockWidget
{
    Q_OBJECT

public:
    explicit FileSystemBrowser(QWidget *parent = nullptr);
    ~FileSystemBrowser();

private:
    Ui::FileSystemBrowser *ui;
};

#endif // FILESYSTEMBROWSER_H
