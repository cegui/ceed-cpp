#ifndef ACTIONCATEGORY_H
#define ACTIONCATEGORY_H

#include "qstring.h"
#include <vector>

// A group of actions

class Action;

class ActionCategory
{
public:

    ActionCategory(const QString& name, const QString& label = QString());
    ~ActionCategory();

    Action* getAction(const QString& name) const;
    void setEnabled(bool enabled);

protected:

    QString _name;
    QString _label;

    std::vector<Action*> _actions;
};

#endif // ACTIONCATEGORY_H
