#include "src/util/Action.h"

Action::Action(ActionCategory& category, const QString& name, QString label,
               const QString& help, QIcon icon, QKeySequence defaultShortcut,
               QString settingsLabel, QAction::MenuRole menuRole)
    : _name(name)
{
    if (label.isEmpty()) label = name;

    /*
            if label is None:
                label = name
            if settingsLabel is None:
                # remove trailing ellipsis and mnemonics
                settingsLabel = label.rstrip(".").replace("&&", "%amp%").replace("&", "").replace("%amp%", "&&")

            self.category = category
            self.defaultShortcut = defaultShortcut
            self.settingsLabel = settingsLabel

            # QAction needs a QWidget parent, we use the main window as that
            super(Action, self).__init__(icon, label, self.getManager().mainWindow)

            self.setToolTip(settingsLabel)
            self.setStatusTip(help_)
            self.setMenuRole(menuRole)

            # we default to application shortcuts and we make sure we always disable the ones we don't need
            # you can override this for individual actions via the setShortcutContext method as seen here
            self.setShortcutContext(Qt.ApplicationShortcut)
            self.setShortcut(defaultShortcut)

            section = self.category.settingsSection

            self.settingsEntry = section.createEntry(name = "shortcut_%s" % (self.name), type_ = QKeySequence, label = "%s" % (self.settingsLabel),
                                                  defaultValue = self.defaultShortcut, widgetHint = "keySequence")

            # when the entry changes, we want to change our shortcut too!
            self.settingsEntry.subscribe(self.setShortcut)
    */
}
