#include "src/util/SettingsCategory.h"

SettingsCategory::SettingsCategory(Settings& settings, const QString& name, const QString& label)
    : _settings(settings)
    ,_name(name);
{

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

    def getSection(self, name):
        for section in self.sections:
            if section.name == name:
                return section

        raise RuntimeError("Section '" + name + "' not found in category '" + self.name + "' of this settings")

    def createEntry(self, **kwargs):
        if self.getSection("") is None:
            section = self.createSection(name = "")
            section.sortingWeight = -1

        section = self.getSection("")
        return section.createEntry(**kwargs)

    def getEntry(self, path):
        # FIXME: Needs better error handling
        splitted = path.split("/", 1)
        assert(len(splitted) == 2)

        section = self.getSection(splitted[0])
        return section.getEntry(splitted[1])

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
