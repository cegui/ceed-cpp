#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include <QDockWidget>
#include "qfilesystemmodel.h"

// This class represents the file system browser dock widget, usually located right bottom
// in the main window. It can browse your entire filesystem and if you double click a file
// it will open an editor tab for it.

namespace Ui {
class FileSystemBrowser;
}

class FileSystemBrowser : public QDockWidget
{
    Q_OBJECT

public:

    explicit FileSystemBrowser(QWidget *parent = nullptr);
    ~FileSystemBrowser();

    void setDirectory(const QString& dir);

signals:

    void fileOpenRequested(const QString& absPath);

private slots:

    void on_view_doubleClicked(const QModelIndex &index);

    void on_parentDirectoryButton_pressed();

    void on_projectDirectoryButton_pressed();

    void on_homeDirectoryButton_pressed();

    void on_activeFileDirectoryButton_pressed();

    void on_pathBox_currentIndexChanged(int index);

private:

    Ui::FileSystemBrowser *ui;

    QFileSystemModel model;
    QString directory;
};

#endif // FILESYSTEMBROWSER_H
