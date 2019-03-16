#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"

Settings::Settings(QSettings* qsettings)
    : _qsettings(qsettings)
{
/*
self.changesRequireRestart = False //???hack flag?
*/
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
/*
    # FIXME: Needs better error handling
    splitted = path.split("/", 1)
    assert(len(splitted) == 2)

    category = self.getCategory(splitted[0])
    return category.getEntry(splitted[1])
*/
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
