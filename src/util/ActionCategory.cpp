#include "src/util/ActionCategory.h"
#include "src/util/ConfigurableAction.h"

ActionCategory::ActionCategory(const QString& name, const QString& label)
    : _name(name)
    , _label(label.isEmpty() ? name : label)
{
/*
        category = self.getManager().settingsCategory
        self.settingsSection = category.createSection(name = self.name, label = self.label)
*/
}

ActionCategory::~ActionCategory()
{
}

Action* ActionCategory::getAction(const QString& name) const
{
    auto it = std::find_if(_actions.begin(), _actions.end(), [&name](const Action* action)
    {
        return action->getName() == name;
    });

    assert(it != _actions.end());
    return (it != _actions.end()) ? (*it) : nullptr;
}

// Allows you to enable/disable actions en masse, especially useful when editors are switched.
// This gets rid of shortcut clashes and so on.
void ActionCategory::setEnabled(bool enabled)
{
    for (auto& action : _actions)
        action->setEnabled(enabled);
}

/*
    def createAction(self, **kwargs):
        action = Action(self, **kwargs)
        self.actions.append(action)

        return action
*/
