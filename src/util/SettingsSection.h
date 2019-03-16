#ifndef SETTINGSSECTION_H
#define SETTINGSSECTION_H

#include "qstring.h"
#include "vector"

// Groups entries, is usually represented by a group box in the interface

class SettingsCategory;
typedef std::unique_ptr<class SettingsEntry> SettingsEntryPtr;

class SettingsSection
{
public:

    SettingsSection(SettingsCategory& category, const QString& name, const QString& label, int sortingWeight = 0);
    ~SettingsSection();

    SettingsEntry* addEntry(SettingsEntryPtr&& entry);
    SettingsEntry* getEntry(const QString& name) const;

    const QString& getName() const { return _name; }
    QString getPath() const;
    SettingsCategory& getCategory() const { return _category; }
    int getSortingWeight() const { return _sortingWeight; }

    void applyChanges();
    void discardChanges();
    void load();
    void store();

    void sort();

protected:

    SettingsCategory& _category;
    QString _name;
    QString _label;
    int _sortingWeight = 0;

    std::vector<SettingsEntryPtr> entries;
};

#endif // SETTINGSSECTION_H
