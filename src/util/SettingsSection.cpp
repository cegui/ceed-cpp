#include "src/util/SettingsSection.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsEntry.h"

// also there was "sortingWeight = 0"
SettingsSection::SettingsSection(SettingsCategory& category, const QString& name, const QString& label)
    : _category(category)
    , _name(name)
    , _label(label.isEmpty() ? name : label)
{

}

SettingsSection::~SettingsSection()
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

QString SettingsSection::getPath() const
{
    return _category.getPath() + "/" + _name;
}

/*
    def createEntry(self, **kwargs):
        entry = Entry(section = self, **kwargs)
        self.entries.append(entry)

        return entry

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
