#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"
#include "src/editors/imageset/ImagesetEditor.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/looknfeel/LookNFeelEditor.h"
#include "qsplashscreen.h"
#include "qsettings.h"
#include "qdir.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    // TODO: move some of this to config file?
    setOrganizationName("CEGUI");
    setOrganizationDomain("cegui.org.uk");
    setApplicationName("CEED - CEGUI editor");
    setApplicationVersion("1.0.0-beta");

    // Create settings and load all values from the persistence store
    settings = new Settings(new QSettings("CEGUI", "CEED", this));
    createSettingsEntries();

    // Finally read stored values into our new setting entries
    settings->load();

    bool showSplash = settings->getEntryValue("global/app/show_splash").toBool();

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

    mainWindow = new MainWindow();
    mainWindow->show();
    mainWindow->raise();

    if (splash)
    {
        splash->finish(mainWindow);
        delete splash;
    }

    /*
        self.errorHandler = ceed.error.ErrorHandler(self.mainWindow)
        self.errorHandler.installExceptionHook()
    */
}

// The absolute path to the doc directory
QString Application::getDocumentationPath() const
{
    /*
    # Potential system doc dir, we check it's existence and set
    # DOC_DIR as system_data_dir if it exists
    SYSTEM_DOC_DIR = "/usr/share/doc/ceed-%s" % (version.CEED)
    SYSTEM_DOC_DIR_EXISTS = False
    try:
        if os.path.exists(SYSTEM_DOC_DIR):
            DOC_DIR = SYSTEM_DOC_DIR
            SYSTEM_DOC_DIR_EXISTS = True

        else:
            SYSTEM_DOC_DIR = "/usr/share/doc/ceed"
            if os.path.exists(SYSTEM_DOC_DIR):
                DOC_DIR = SYSTEM_DOC_DIR
                SYSTEM_DOC_DIR_EXISTS = True
    */

    //!!!must be a working directory, not exe path! or move exe to ROOT/bin & add '../' to the doc subpath?
    //!!!need to determine package path. On windows it can be built from exe path, like EXE_PATH/../ (given exe is in ROOT/bin)
    return QDir::current().absoluteFilePath("doc");
}

// Creates general application settings plus some subsystem settings
// TODO: subsystem settings to subsystems? load from storage on subsystem init?
void Application::createSettingsEntries()
{
    // General settings

    auto catGlobal = settings->createCategory("global", "Global");
    auto secUndoRedo = catGlobal->createSection("undo", "Undo and Redo");

    // By default we limit the undo stack to 500 undo commands, should be enough and should
    // avoid memory drainage. keep in mind that every tabbed editor has it's own undo stack,
    // so the overall command limit is number_of_tabs * 500!
    SettingsEntryPtr entry(new SettingsEntry(*secUndoRedo, "limit", 500, "Limit (number of steps)",
                                             "Puts a limit on every tabbed editor's undo stack. You can undo at most the number of times specified here.",
                                             "int", true, 1));
    secUndoRedo->addEntry(std::move(entry));

    auto secApp = catGlobal->createSection("app", "Application");
    entry.reset(new SettingsEntry(*secApp, "show_splash", true, "Show splash screen",
                                  "Show the splash screen on startup",
                                  "checkbox", false, 1));
    secApp->addEntry(std::move(entry));

    auto secUI = catGlobal->createSection("ui", "User Interface");
    entry.reset(new SettingsEntry(*secUI, "toolbar_icon_size", 32, "Toolbar icon size",
                                  "Sets the size of the toolbar icons",
                                  "combobox", false, 1, { {32, "Normal"}, {24, "Small"}, {16, "Smaller"} }));
    secUI->addEntry(std::move(entry));

    auto secCEGUIDebug = catGlobal->createSection("cegui_debug_info", "CEGUI debug info");
    entry.reset(new SettingsEntry(*secCEGUIDebug, "log_limit", 20000, "Log messages limit",
                                  "Limits number of remembered log messages to given amount. This is there to prevent endless growth of memory consumed by CEED.",
                                  "int", true, 1));
    secCEGUIDebug->addEntry(std::move(entry));

    auto secNavigation = catGlobal->createSection("navigation", "Navigation");
    entry.reset(new SettingsEntry(*secNavigation, "ctrl_zoom", true, "Only zoom when CTRL is pressed",
                                  "Mouse wheel zoom is ignored unless the Control key is pressed when it happens.",
                                  "checkbox", false, 1));
    secNavigation->addEntry(std::move(entry));

    // CEGUI settings

    auto catCEGUI = settings->createCategory("cegui", "Embedded CEGUI");
    auto secBG = catCEGUI->createSection("background", "Rendering background (checkerboard)");

    entry.reset(new SettingsEntry(*secBG, "checker_width", 10, "Width of the checkers",
                                  "Width of one checker element in pixels.",
                                  "int", false, 1));
    secBG->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secBG, "checker_height", 10, "Height of the checkers",
                                  "Height of one checker element in pixels.",
                                  "int", false, 2));
    secBG->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secBG, "first_colour", QColor(Qt::darkGray), "First colour",
                                  "First of the alternating colours to use.",
                                  "colour", false, 3));
    secBG->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secBG, "second_colour", QColor(Qt::lightGray), "Second colour",
                                  "Second of the alternating colours to use. (use the same as first to get a solid background)",
                                  "colour", false, 4));
    secBG->addEntry(std::move(entry));

    ImagesetEditor::createSettings(*settings);
    LayoutEditor::createSettings(*settings);
    LookNFeelEditor::createSettings(*settings);
}
