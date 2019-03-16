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

void SettingsSection::applyChanges()
{
    for (auto&& entry : entries)
        entry->applyChanges();
}

void SettingsSection::discardChanges()
{
    for (auto&& entry : entries)
        entry->discardChanges();
}

void SettingsSection::load()
{
    for (auto&& entry : entries)
        entry->load();
}

void SettingsSection::store()
{
    for (auto&& entry : entries)
        entry->store();
}

void SettingsSection::sort()
{
    std::sort(entries.begin(), entries.end(), [](const SettingsEntryPtr& a, const SettingsEntryPtr& b)
    {
        //return a->getName() < b->getName();
        return a->getSortingWeight() < b->getSortingWeight();
    });
}

/*
    def createEntry(self, **kwargs):
        entry = Entry(section = self, **kwargs)
        self.entries.append(entry)

        return entry
*/
