#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "qstringlist.h"

Settings::Settings(QSettings* qsettings)
    : _qsettings(qsettings) //???here or in app as back-end? access from entries through a singleton?
{
/*
self.changesRequireRestart = False //???hack flag?
*/
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

/*
    def markRequiresRestart(self):
        self.changesRequireRestart = True

    def applyChanges(self):
        for category in self.categories:
            category.applyChanges()

    def discardChanges(self):
        for category in self.categories:
            category.discardChanges()

    def upload(self):
        for category in self.categories:
            category.upload()

    def download(self):
        for category in self.categories:
            category.download()

        self.changesRequireRestart = False

    def sort(self, recursive = True):
        # FIXME: This is obviously not the fastest approach
        self.categories = sorted(self.categories, key = lambda category: category.name)
        self.categories = sorted(self.categories, key = lambda category: category.sortingWeight)

        if recursive:
            for category in self.categories:
                category.sort()
*/
/*
    def upload(self, entry, value):
        self.qsettings.setValue(entry.getPath(), value)

    def download(self, entry):
        return self.qsettings.value(entry.getPath())
*/
