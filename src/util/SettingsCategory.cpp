#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/Settings.h"
#include "qstringlist.h"

// also there was "sortingWeight = 0" but it wasn't used
SettingsCategory::SettingsCategory(Settings& settings, const QString& name, const QString& label, int sortingWeight)
    : _settings(settings)
    ,_name(name)
    , _label(label.isEmpty() ? name : label)
    , _sortingWeight(sortingWeight)
{
}

SettingsCategory::~SettingsCategory()
{
}

SettingsSection* SettingsCategory::createSection(const QString& name, const QString& label, int sortingWeight)
{
    SettingsSection* section = new SettingsSection(*this, name, label, sortingWeight);
    sections.push_back(SettingsSectionPtr(section));
    return section;
}

SettingsSection* SettingsCategory::getSection(const QString& name) const
{
    auto it = std::find_if(sections.begin(), sections.end(), [&name](const SettingsSectionPtr& sec)
    {
        return sec->getName() == name;
    });

    return (it != sections.end()) ? it->get() : nullptr;
}

// It was createEntry(), and probably that was better from the OOP point,
// but SettingsEntry has so many constructor args that I don't want to
// copy them here and keep them in sync. Let it be compact and readable.
SettingsEntry* SettingsCategory::addEntry(SettingsEntryPtr&& entry)
{
    auto section = getSection("");
    if (!section)
        section = createSection("", "", -1);

    return section->addEntry(std::move(entry));
}

SettingsEntry* SettingsCategory::getEntry(const QString& path) const
{
    return getEntry(path.split("/"));
}

SettingsEntry* SettingsCategory::getEntry(const QStringList& pathSplitted) const
{
    if (pathSplitted.size() < 2) return nullptr;
    auto section = getSection(pathSplitted[0]);
    return section ? section->getEntry(pathSplitted[1]) : nullptr;
}

QString SettingsCategory::getPath() const
{
    return _settings.getPath() + "/" + _name;
}

void SettingsCategory::applyChanges()
{
    for (auto&& section : sections)
        section->applyChanges();
}

void SettingsCategory::discardChanges()
{
    for (auto&& section : sections)
        section->discardChanges();
}

void SettingsCategory::load()
{
    for (auto&& section : sections)
        section->load();
}

void SettingsCategory::store()
{
    for (auto&& section : sections)
        section->store();
}

void SettingsCategory::sort(bool deep)
{
    std::sort(sections.begin(), sections.end(), [](const SettingsSectionPtr& a, const SettingsSectionPtr& b)
    {
        // Weight, then name
        if (a->getSortingWeight() != b->getSortingWeight())
            return a->getSortingWeight() < b->getSortingWeight();
        else
            return a->getName() < b->getName();
    });

    if (deep)
    {
        for (auto&& section : sections)
            section->sort();
    }
}

bool SettingsCategory::isModified() const
{
    for (auto&& section : sections)
        if (section->isModified()) return true;
    return false;
}
