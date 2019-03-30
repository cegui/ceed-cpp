#include "src/util/ConfigurableAction.h"
#include "src/Application.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"

ConfigurableAction::ConfigurableAction(QWidget* parent, SettingsSection& section, const QString& name, QString label,
                                       const QString& help, QIcon icon, QKeySequence defaultShortcut,
                                       Qt::ShortcutContext shortcutContext, QString settingsLabel, QAction::MenuRole menuRole)
    : QAction(parent)
{
    if (label.isEmpty()) label = name;

    if (settingsLabel.isEmpty())
        settingsLabel = label.mid(0, settingsLabel.lastIndexOf(".")).replace("&&", "%amp%").replace("&", "").replace("%amp%", "&&");

    setObjectName(name);
    setText(label);
    setIcon(icon);
    setToolTip(settingsLabel);
    setStatusTip(help);
    setMenuRole(menuRole);
    setShortcutContext(shortcutContext);
    setShortcut(defaultShortcut);

    SettingsEntryPtr entryPtr(new SettingsEntry(section, name, defaultShortcut, settingsLabel, help, "keySequence", false, 1));
    auto entry = section.addEntry(std::move(entryPtr));

    // When the entry changes, we want to change our shortcut too!
    connect(entry, &SettingsEntry::valueChanged, [this](const QVariant& value)
    {
        setShortcut(value.value<QKeySequence>());
    });
}
