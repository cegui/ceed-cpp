#include "src/util/SettingsCategory.h"
#include "qstringlist.h"
#include "src/util/SettingsSection.h"

SettingsCategory::SettingsCategory(Settings& settings, const QString& name, const QString& label)
    : _settings(settings)
    ,_name(name)
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

/*
    def __init__(self, settings, name, label = None, sortingWeight = 0):
        self.settings = settings

        if label is None:
            label = name

        self.label = label
        self.sortingWeight = sortingWeight

        self.sections = []

    def getPath(self):
        """Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example
        """

        return "%s/%s" % (self.settings.getPath(), self.name)

    def getPersistenceProvider(self):
        return self.settings.getPersistenceProvider()

    def getSettings(self):
        return self.settings

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

    def markAsChanged(self):
        if not self.label.startswith("* "):
            self.label = " ".join(["*", self.label])

    def markAsUnchanged(self):
        if self.label.startswith("* "):
            self.label = self.label[2:]

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
