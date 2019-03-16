#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "qstringlist.h"

Settings::Settings(QSettings* qsettings)
    : _qsettings(qsettings) //???here or in app as back-end? access from entries through a singleton?
{
}

Settings::~Settings()
{
}

SettingsCategory* Settings::createCategory(const QString& name, const QString& label)
{
    SettingsCategory* category = new SettingsCategory(*this, name, label);
    categories.push_back(SettingsCategoryPtr(category));
    return category;
}

SettingsCategory* Settings::getCategory(const QString& name) const
{
    auto it = std::find_if(categories.begin(), categories.end(), [&name](const SettingsCategoryPtr& cat)
    {
        return cat->getName() == name;
    });

    assert(it != categories.end());
    return (it != categories.end()) ? it->get() : nullptr;
}

SettingsEntry* Settings::getEntry(const QString& path) const
{
    return getEntry(path.split("/"));
}

SettingsEntry* Settings::getEntry(QStringList pathSplitted) const
{
    if (pathSplitted.size() < 2) return nullptr;
    auto category = getCategory(pathSplitted[0]);
    pathSplitted.pop_front();
    return category ? category->getEntry(pathSplitted) : nullptr;
}

void Settings::applyChanges()
{
    for (auto&& category : categories)
        category->applyChanges();
}

void Settings::discardChanges()
{
    for (auto&& category : categories)
        category->discardChanges();
}

void Settings::load()
{
    for (auto&& category : categories)
        category->load();

    _changesRequireRestart = false;
}

void Settings::store()
{
    for (auto&& category : categories)
        category->store();
}

void Settings::sort(bool deep)
{
    std::sort(categories.begin(), categories.end(), [](const SettingsCategoryPtr& a, const SettingsCategoryPtr& b)
    {
        //return a->getName() < b->getName();
        return a->getSortingWeight() < b->getSortingWeight();
    });

    if (deep)
    {
        for (auto&& category : categories)
            category->sort();
    }
}
