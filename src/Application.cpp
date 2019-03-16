#include "src/Application.h"
#include "qsplashscreen.h"
#include "qsettings.h"
#include "src/util/Settings.h"

Application::Application(int& argc, char** argv, bool debug)
    : QApplication(argc, argv)
{
    setOrganizationName("CEGUI");
    setOrganizationDomain("cegui.org.uk");
    setApplicationName("CEED - CEGUI editor");
    setApplicationVersion("0.0.0-master"); // TODO: move to file?

/*
        logging.basicConfig()

        if debug:
            logging.getLogger().setLevel(logging.DEBUG)
*/

    QSettings* qsettings = new QSettings("CEGUI", "CEED", this);
    settings = new Settings(qsettings);
/*
        # download all values from the persistence store
        self.settings.download()

        showSplash = settings.getEntry("global/app/show_splash").value
*/
    bool showSplash = true; // TODO: read setting!

    QSplashScreen* splash = nullptr;
    if (showSplash)
    {
        splash = new QSplashScreen(QPixmap(":/images/splashscreen.png"));
        splash->setWindowModality(Qt::ApplicationModal);
        splash->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
        splash->showMessage("version: " + applicationVersion(), Qt::AlignTop | Qt::AlignRight, Qt::GlobalColor::white);

        splash->show();

        // This ensures that the splash screen is shown on all platforms
        processEvents();
    }

    mainWindow.show();
    mainWindow.raise();

    if (splash)
    {
        splash->finish(&mainWindow);
        delete splash;
    }

    /*
        self.errorHandler = ceed.error.ErrorHandler(self.mainWindow)
        self.errorHandler.installExceptionHook()
    */
}
