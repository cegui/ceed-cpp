#ifndef APPLICATION_H
#define APPLICATION_H

#include "qapplication.h"
#include "ui/MainWindow.h"

// The central application class

class Settings;

class Application : public QApplication
{
    Q_OBJECT

public:

    Application(int& argc, char** argv, bool debug = false);

    MainWindow* getMainWindow() { return &mainWindow; }
    Settings* getSettings() const { return settings; }

private:

    MainWindow mainWindow;
    Settings* settings = nullptr;
};

#endif // APPLICATION_H
