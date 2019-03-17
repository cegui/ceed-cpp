#ifndef SETTINGSCATEGORY_H
#define SETTINGSCATEGORY_H

#include "qstring.h"
#include "vector"

// Groups sections, is usually represented by a tab in the interface

class Settings;
typedef std::unique_ptr<class SettingsEntry> SettingsEntryPtr;
typedef std::unique_ptr<class SettingsSection> SettingsSectionPtr;

class SettingsCategory
{
public:

    SettingsCategory(Settings& settings, const QString& name, const QString& label, int sortingWeight = 0);
    ~SettingsCategory();

    SettingsSection* createSection(const QString& name, const QString& label, int sortingWeight = 0);
    SettingsSection* getSection(const QString& name) const;
    const std::vector<SettingsSectionPtr>& getSections() const { return sections; }

    SettingsEntry* addEntry(SettingsEntryPtr&& entry);
    SettingsEntry* getEntry(const QString& path) const;
    SettingsEntry* getEntry(const QStringList& pathSplitted) const;

    const QString& getName() const { return _name; }
    QString getLabel() const { return (isModified() ? "* " : "") + _label; }
    QString getPath() const;
    Settings& getSettings() const { return _settings; }
    int getSortingWeight() const { return _sortingWeight; }

    bool isModified() const;

    void applyChanges();
    void discardChanges();
    void load();
    void store();

    void sort(bool deep = true);

protected:

    Settings& _settings;
    QString _name;
    QString _label;
    int _sortingWeight = 0;

    std::vector<SettingsSectionPtr> sections;
};

#endif // SETTINGSCATEGORY_H
