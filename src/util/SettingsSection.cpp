#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"

SettingsSection::SettingsSection()
{

}

SettingsEntry* SettingsSection::getEntry(const QString& name) const
{
    auto it = std::find_if(entries.begin(), entries.end(), [&name](const SettingsEntryPtr& ent)
    {
        return ent->getName() == name;
    });

    assert(it != entries.end());
    return (it != entries.end()) ? it->get() : nullptr;
}

/*
    def __init__(self, category, name, label = None, sortingWeight = 0):
        self.category = category

        if label is None:
            label = name

        self.name = name
        self.label = label
        self.sortingWeight = sortingWeight

        self.entries = []

    def getPath(self):
        """Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example
        """

        return "%s/%s" % (self.category.getPath(), self.name)

    def getPersistenceProvider(self):
        return self.category.getPersistenceProvider()

    def getSettings(self):
        return self.category.getSettings()

    def createEntry(self, **kwargs):
        entry = Entry(section = self, **kwargs)
        self.entries.append(entry)

        return entry

    # - Reserved for possible future use.
    def markAsChanged(self):
        pass

    def markAsUnchanged(self):
        pass

    def applyChanges(self):
        for entry in self.entries:
            entry.applyChanges()

    def discardChanges(self):
        for entry in self.entries:
            entry.discardChanges()

    def upload(self):
        for entry in self.entries:
            entry.upload()

    def download(self):
        for entry in self.entries:
            entry.download()

    def sort(self):
        # FIXME: This is obviously not the fastest approach
        self.entries = sorted(self.entries, key = lambda entry: entry.name)
        self.entries = sorted(self.entries, key = lambda entry: entry.sortingWeight)
*/
