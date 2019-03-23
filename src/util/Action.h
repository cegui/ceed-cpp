#ifndef ACTION_H
#define ACTION_H

#include "qaction.h"

// The only thing different in this from QAction is the ability to change the shortcut of it
// using CEED's settings API/interface.
// While it isn't needed/required to use this everywhere where QAction is used, it is recommended.

class ActionCategory;

class Action : public QAction
{
public:

    Action(ActionCategory& category, const QString& name, QString label = QString(),
           const QString& help = QString(), QIcon icon = QIcon(), QKeySequence defaultShortcut = QKeySequence(),
           QString settingsLabel = QString(), QAction::MenuRole menuRole = QAction::TextHeuristicRole);

    const QString& getName() const { return _name; }

protected:

    QString _name;
};

#endif // ACTION_H
