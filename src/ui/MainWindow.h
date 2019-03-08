#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ProjectManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();

    void on_actionStatusbar_toggled(bool arg1);

private:

    Ui::MainWindow* ui;
    ProjectManager* projectManager = nullptr;
};

#endif // MAINWINDOW_H
