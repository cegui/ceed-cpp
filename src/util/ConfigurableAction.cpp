#include "src/util/ConfigurableAction.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"

ConfigurableAction::ConfigurableAction(QWidget* parent, const QString& name, QString label, const QString& help,
                                       QIcon icon, QKeySequence defaultShortcut, Qt::ShortcutContext shortcutContext,
                                       QString settingsLabel, QAction::MenuRole menuRole)
    : QAction(parent)
{
    if (label.isEmpty()) label = name;

    if (settingsLabel.isEmpty())
    {
        settingsLabel = settingsLabel.mid(0, settingsLabel.lastIndexOf(".")).replace("&&", "%amp%").replace("&", "").replace("%amp%", "&&");
    }

    setObjectName(name);
    setText(label);
    setIcon(icon);
    setToolTip(settingsLabel);
    setStatusTip(help);
    setMenuRole(menuRole);
    setShortcutContext(shortcutContext);
    setShortcut(defaultShortcut);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    auto category = settings->getCategory("shortcuts");
    if (!category) category = settings->createCategory("shortcuts", "Shortcuts");

    // TODO: pass settings section to the constructor
    auto section = category->createSection("FILL_ME", "FILL_ME");

    // By default we limit the undo stack to 500 undo commands, should be enough and should
    // avoid memory drainage. keep in mind that every tabbed editor has it's own undo stack,
    // so the overall command limit is number_of_tabs * 500!
    SettingsEntryPtr entry(new SettingsEntry(*section, name, defaultShortcut, settingsLabel,
                                             help, "keySequence", false, 1));
    section->addEntry(std::move(entry));

    // When the entry changes, we want to change our shortcut too!
    connect(entry.get(), &SettingsEntry::valueChanged, [this](const QVariant& value)
    {
        setShortcut(value.value<QKeySequence>());
    });
}
