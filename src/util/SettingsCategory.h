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
    SettingsEntry* addEntry(SettingsEntryPtr&& entry);
    SettingsEntry* getEntry(const QString& path) const;
    SettingsEntry* getEntry(const QStringList& pathSplitted) const;

    void setModified(bool modified) { _changed = modified; }

    const QString& getName() const { return _name; }
    QString getLabel() const { return (_changed ? "* " : "") + _label; }
    QString getPath() const;
    Settings& getSettings() const { return _settings; }
    int getSortingWeight() const { return _sortingWeight; }

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

    bool _changed = false;
};

#endif // SETTINGSCATEGORY_H
