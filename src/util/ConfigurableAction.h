#ifndef ACTION_H
#define ACTION_H

#include "qaction.h"

// The only thing different in this from QAction is the ability to change the shortcut of it
// using CEED's settings API/interface.
// While it isn't needed/required to use this everywhere where QAction is used, it is recommended.
// NB: we are defaulting to WindowShortcut because Widget[WithChildren]Shortcut doesn't work with
// a graphics view for some reason.

class SettingsSection;

class ConfigurableAction : public QAction
{
public:

    ConfigurableAction(QWidget* parent, SettingsSection& section, const QString& name, QString label = QString(), const QString& help = QString(),
                       QIcon icon = QIcon(), QKeySequence defaultShortcut = QKeySequence(), Qt::ShortcutContext shortcutContext = Qt::WindowShortcut,
                       QString settingsLabel = QString(), QAction::MenuRole menuRole = QAction::TextHeuristicRole);
};

#endif // ACTION_H
