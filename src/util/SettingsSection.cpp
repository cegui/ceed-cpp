#include "src/util/SettingsSection.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsEntry.h"

// also there was "sortingWeight = 0"
SettingsSection::SettingsSection(SettingsCategory& category, const QString& name, const QString& label, int sortingWeight)
    : _category(category)
    , _name(name)
    , _label(label.isEmpty() ? name : label)
    , _sortingWeight(sortingWeight)
{

}

SettingsSection::~SettingsSection()
{
}

// It was createEntry(), and probably that was better from the OOP point,
// but SettingsEntry has so many constructor args that I don't want to
// copy them here and keep them in sync. Let it be compact and readable.
SettingsEntry* SettingsSection::addEntry(SettingsEntryPtr&& entry)
{
    entries.push_back(std::move(entry));
    return entries.back().get();
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
