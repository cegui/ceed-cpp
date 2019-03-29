#ifndef ACTIONCATEGORY_H
#define ACTIONCATEGORY_H

#include "qstring.h"
#include <vector>

// A group of actions

class ConfigurableAction;

class ActionCategory
{
public:

    ActionCategory(const QString& name, const QString& label = QString());
    ~ActionCategory();

    ConfigurableAction* getAction(const QString& name) const;
    void setEnabled(bool enabled);

protected:

    QString _name;
    QString _label;

    std::vector<ConfigurableAction*> _actions;
};

#endif // ACTIONCATEGORY_H
