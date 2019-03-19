#include "src/util/RecentlyUsed.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "qsettings.h"

RecentlyUsed::RecentlyUsed(const QString& sectionID, QObject* parent)
    : QObject(parent)
    , _sectionID("recentlyUsedIdentifier/" + sectionID)
{
}

void RecentlyUsed::addRecentlyUsed(const QString& name)
{
    QStringList items;
    getRecentlyUsed(items);

    if (!items.contains(name))
        items.insert(0, name);

    while (items.size() > _maxItems)
        items.pop_back();

    auto settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    settings->setValue(_sectionID, items);
}

// Safe to call even if the item is not in the list
bool RecentlyUsed::removeRecentlyUsed(const QString& name)
{
    QStringList items;
    getRecentlyUsed(items);

    if (!items.contains(name)) return false;

    items.removeAll(name);
    auto settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    settings->setValue(_sectionID, items);
    return true;
}

void RecentlyUsed::clearRecentlyUsed()
{
    auto settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    settings->remove(_sectionID);
}

void RecentlyUsed::getRecentlyUsed(QStringList& outList) const
{
    auto settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    if (!settings->contains(_sectionID)) return;

    outList = settings->value(_sectionID).toStringList();
}

//---------------------------------------------------------------------

RecentlyUsedMenuEntry::RecentlyUsedMenuEntry(const QString& sectionID, QObject* parent)
    : RecentlyUsed(sectionID, parent)
{
}

void RecentlyUsedMenuEntry::setParentMenu(QMenu* menu)
{
    if (_menu == menu) return;
    _menu = menu;
    updateMenu();
}

void RecentlyUsedMenuEntry::addRecentlyUsed(const QString& name)
{
    RecentlyUsed::addRecentlyUsed(name);
    updateMenu();
}

bool RecentlyUsedMenuEntry::removeRecentlyUsed(const QString& name)
{
    const bool result = RecentlyUsed::removeRecentlyUsed(name);
    updateMenu();
    return result;
}

void RecentlyUsedMenuEntry::clearRecentlyUsed()
{
    RecentlyUsed::clearRecentlyUsed();
    updateMenu();
}

void RecentlyUsedMenuEntry::recentItemActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
        emit triggered(action->data().toString());
}

void RecentlyUsedMenuEntry::updateMenu()
{
    if (!_menu) return;

    _menu->clear();

    QStringList items;
    getRecentlyUsed(items);

    int i = 0;
    for (auto& item : items)
    {
        QString text = item;

        constexpr int _maxNameLength = 60;
        if (text.length() > _maxNameLength)
            text = "..." + text.right(_maxNameLength - 3);

        if (i < 10)
            text = QString("&%1. %2").arg((i + 1) % 10).arg(text);

        auto action = new QAction(text, _menu);
        action->setData(item);
        action->setVisible(true);
        _menu->addAction(action);
        connect(action, &QAction::triggered, this, &RecentlyUsedMenuEntry::recentItemActionTriggered);

        ++i;
    }

    _menu->addSeparator();

    auto action = new QAction("Clear", _menu);
    action->setVisible(true);
    action->setEnabled(!items.empty());
    _menu->addAction(action);
    connect(action, &QAction::triggered, this, &RecentlyUsedMenuEntry::clearRecentlyUsed);
}
