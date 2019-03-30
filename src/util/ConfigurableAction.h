#ifndef ACTION_H
#define ACTION_H

#include "qaction.h"

// The only thing different in this from QAction is the ability to change the shortcut of it
// using CEED's settings API/interface.
// While it isn't needed/required to use this everywhere where QAction is used, it is recommended.

class SettingsSection;

class ConfigurableAction : public QAction
{
public:

    ConfigurableAction(QWidget* parent, SettingsSection& section, const QString& name, QString label = QString(), const QString& help = QString(),
                       QIcon icon = QIcon(), QKeySequence defaultShortcut = QKeySequence(), Qt::ShortcutContext shortcutContext = Qt::WidgetWithChildrenShortcut,
                       QString settingsLabel = QString(), QAction::MenuRole menuRole = QAction::TextHeuristicRole);
};

#endif // ACTION_H
