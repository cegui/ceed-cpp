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

    SettingsSection(SettingsCategory& category, const QString& name, const QString& label);
    ~SettingsSection();

    SettingsEntry* getEntry(const QString& name) const;

    const QString& getName() const { return _name; }
    QString getPath() const;
    SettingsCategory& getCategory() const { return _category; }

protected:

    SettingsCategory& _category;
    QString _name;
    QString _label;
    std::vector<SettingsEntryPtr> entries;
};

#endif // SETTINGSSECTION_H
