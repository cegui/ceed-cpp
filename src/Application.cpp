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
#include <qaction.h>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setOrganizationName("CEGUI");
    setOrganizationDomain("cegui.org.uk");
    setApplicationName("CEED - CEGUI editor");
    setApplicationVersion("1.0.0");

    // Create settings and load all values from the persistence store
    _settings = new Settings(new QSettings("CEGUI", "CEED", this));
    createSettingsEntries();
    ImagesetEditor::createActions(*this);
    LayoutEditor::createActions(*this);

    // Finally read stored values into our new setting entries
    _settings->load();

    QSplashScreen* splash = nullptr;
    if (_settings->getEntryValue("global/app/show_splash").toBool())
    {
        splash = new QSplashScreen(QPixmap(":/images/splashscreen.png"));
        splash->setWindowModality(Qt::ApplicationModal);
        splash->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
        splash->showMessage("version: " + applicationVersion(), Qt::AlignTop | Qt::AlignRight, Qt::GlobalColor::white);

        splash->show();

        // This ensures that the splash screen is shown on all platforms
        processEvents();
    }

    _mainWindow = new MainWindow();

    ImagesetEditor::createToolbar(*this);
    LayoutEditor::createToolbar(*this);

    _mainWindow->show();
    _mainWindow->raise();

    if (splash)
    {
        splash->finish(_mainWindow);
        delete splash;
    }

    // Perform a startup action
    const auto action = _settings->getEntryValue("global/app/startup_action").toInt();
    switch (action)
    {
        case 1: _mainWindow->openMostRecentProject(); break;
        default: break; // 0: empty environment
    }
}

Application::~Application()
{
    delete _mainWindow;
    delete _settings;
}

SettingsSection* Application::getOrCreateShortcutSettingsSection(const QString& groupId, const QString& label)
{
    auto category = _settings->getCategory("shortcuts");
    if (!category) category = _settings->createCategory("shortcuts", "Shortcuts");
    auto section = category->getSection(groupId);
    return section ? section : category->createSection(groupId, label);
}

QAction* Application::registerAction(const QString& groupId, const QString& id, const QString& label,
                                     const QString& help, QIcon icon, QKeySequence defaultShortcut, bool checkable)
{
    QString actualLabel = label.isEmpty() ? id : label;
    QString settingsLabel = actualLabel.replace("&&", "%amp%").replace("&", "").replace("%amp%", "&&");

    QAction* action = new QAction(this);
    action->setObjectName(id);
    action->setText(actualLabel);
    action->setIcon(icon);
    action->setToolTip(settingsLabel);
    action->setStatusTip(help);
    //action->setMenuRole(menuRole);
    action->setShortcutContext(Qt::WindowShortcut);
    action->setShortcut(defaultShortcut);
    action->setCheckable(checkable);

    _globalActions.emplace(groupId + "/" + id, action);

    if (!_settings) return action;
    auto category = _settings->getCategory("shortcuts");
    if (!category) return action;
    auto section = category->getSection(groupId);
    if (!section) return action;

    SettingsEntryPtr entryPtr(new SettingsEntry(*section, id, defaultShortcut, settingsLabel, help, "keySequence", false, 1));
    auto entry = section->addEntry(std::move(entryPtr));

    // When the entry changes, we want to change our shortcut too!
    connect(entry, &SettingsEntry::valueChanged, [action](const QVariant& value)
    {
        action->setShortcut(value.value<QKeySequence>());
    });

    return action;
}

QAction* Application::getAction(const QString& fullId) const
{
    auto it = _globalActions.find(fullId);
    return (it == _globalActions.end()) ? nullptr : it->second;
}

void Application::setActionsEnabled(const QString& groupId, bool enabled)
{
    const QString prefix = groupId + "/";
    for (auto& pair : _globalActions)
        if (pair.first.startsWith(prefix))
            pair.second->setEnabled(enabled);
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

    return QDir::current().absoluteFilePath("doc");
}

// Creates general application settings plus some subsystem settings
// TODO: subsystem settings to subsystems? load from storage on subsystem init?
void Application::createSettingsEntries()
{
    // General settings

    auto catGlobal = _settings->createCategory("global", "Global");
    auto secApp = catGlobal->createSection("app", "Application");

    SettingsEntryPtr entry(new SettingsEntry(*secApp, "startup_action", 1, "On startup, load",
                                "What to show when an application started",
                                "combobox", false, 1, { {0, "Empty environment"}, {1, "Most recent project"} }));
    secApp->addEntry(std::move(entry));

    // By default we limit the undo stack to 500 undo commands, should be enough and should
    // avoid memory drainage. Keep in mind that every tabbed editor has it's own undo stack,
    // so the overall command limit is number_of_tabs * 500!
    entry.reset(new SettingsEntry(*secApp, "undo_limit", 500, "Undo history size",
                                 "Puts a limit on every tabbed editor's undo stack. You can undo at most the number of times specified here.",
                                 "int", true, 1));
    secApp->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secApp, "copy_path_os_separators", true, "Copy path with OS-specific separators",
                                  "When copy a file path to clipboard, will convert forward slashes (/) to OS-specific separators",
                                  "checkbox", false, 1));
    secApp->addEntry(std::move(entry));

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

    auto catCEGUI = _settings->createCategory("cegui", "Embedded CEGUI");
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

    auto secScreenshots = catCEGUI->createSection("screenshots", "Screenshots");

    entry.reset(new SettingsEntry(*secScreenshots, "save", true, "Save to file",
                                  "Save screenshot to file (otherwise it is only copied to the clipboard)",
                                  "checkbox", false, 1));
    secScreenshots->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secScreenshots, "after_save_action", 0, "After save",
                                  "What to do after saving a screenshot to the file",
                                  "combobox", false, 2, { {0, "Open folder"}, {1, "Open file"}, {2, "Do nothing"} }));
    secScreenshots->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secScreenshots, "bg_checker", false, "Checkered background in clipboard",
                                  "Fill screenshot background with a checkerboard (otherwise\n"
                                  "transparency is kept when pasting to applications that\n"
                                  "support transparent images)",
                                  "checkbox", false, 3));
    secScreenshots->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secScreenshots, "use_qt_setimage", true, "Add Qt image to clipboard",
                                  "Adds Qt's 'application/x-qt-image' to a clipboard, which\n"
                                  "expands to multiple platform and cross-platform formats.\n"
                                  "On Windows it's known to enable pasting to Paint & Slack,\n"
                                  "but to break pasting to Word.",
                                  "checkbox", false, 4));

    secScreenshots->addEntry(std::move(entry));

    ImagesetEditor::createSettings(*_settings);
    LayoutEditor::createSettings(*_settings);
    LookNFeelEditor::createSettings(*_settings);
}
