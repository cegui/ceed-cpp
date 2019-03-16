#ifndef APPLICATION_H
#define APPLICATION_H

#include "qapplication.h"
#include "ui/MainWindow.h"

// The central application class

class Application : public QApplication
{
public:

    Application(int& argc, char** argv, bool debug = false);

    MainWindow* getMainWindow() { return &mainWindow; }

private:

    MainWindow mainWindow;
};

#endif // APPLICATION_H
