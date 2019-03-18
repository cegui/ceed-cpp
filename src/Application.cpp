#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"
#include "qsplashscreen.h"
#include "qsettings.h"

Application::Application(int& argc, char** argv, bool debug)
    : QApplication(argc, argv)
{
    // TODO: move some of this to config file?
    setOrganizationName("CEGUI");
    setOrganizationDomain("cegui.org.uk");
    setApplicationName("CEED - CEGUI editor");
    setApplicationVersion("0.0.0-master");

/*
        logging.basicConfig()

        if debug:
            logging.getLogger().setLevel(logging.DEBUG)
*/

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

    entry.reset(new SettingsEntry(*secBG, "first_colour", QVariant::fromValue(QColor(Qt::darkGray)), "First colour",
                                  "First of the alternating colours to use.",
                                  "colour", false, 3));
    secBG->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secBG, "second_colour", QVariant::fromValue(QColor(Qt::lightGray)), "Second colour",
                                  "Second of the alternating colours to use. (use the same as first to get a solid background)",
                                  "colour", false, 4));
    secBG->addEntry(std::move(entry));

    //!!!if used only by editors, move to editor factories & call on factories init!
    /*
    import ceed.editors.imageset.settings_decl as imageset_settings
    imageset_settings.declare(self)

    import ceed.editors.layout.settings_decl as layout_settings
    layout_settings.declare(self)

    import ceed.editors.looknfeel.settings_decl as looknfeel_settings
    looknfeel_settings.declare(self)
    */
}
