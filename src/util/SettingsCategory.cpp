#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/Settings.h"
#include "qstringlist.h"

// also there was "sortingWeight = 0"
SettingsCategory::SettingsCategory(Settings& settings, const QString& name, const QString& label)
    : _settings(settings)
    ,_name(name)
    , _label(label.isEmpty() ? name : label)
{
}

SettingsCategory::~SettingsCategory()
{
}

SettingsSection* SettingsCategory::getSection(const QString& name) const
{
    auto it = std::find_if(sections.begin(), sections.end(), [&name](const SettingsSectionPtr& sec)
    {
        return sec->getName() == name;
    });

    assert(it != sections.end());
    return (it != sections.end()) ? it->get() : nullptr;
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

/*
    def createSection(self, **kwargs):
        section = Section(category = self, **kwargs)
        self.sections.append(section)

        return section

    def createEntry(self, **kwargs):
        if self.getSection("") is None:
            section = self.createSection(name = "")
            section.sortingWeight = -1

        section = self.getSection("")
        return section.createEntry(**kwargs)

    def applyChanges(self):
        for section in self.sections:
            section.applyChanges()

    def discardChanges(self):
        for section in self.sections:
            section.discardChanges()

    def upload(self):
        for section in self.sections:
            section.upload()

    def download(self):
        for section in self.sections:
            section.download()

    def sort(self, recursive = True):
        self.sections = sorted(self.sections, key = lambda section: (section.sortingWeight, section.name))

        if recursive:
            for section in self.sections:
                section.sort()
*/
