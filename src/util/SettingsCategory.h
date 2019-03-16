#ifndef SETTINGSCATEGORY_H
#define SETTINGSCATEGORY_H

#include "qstring.h"
#include "vector"

// Groups sections, is usually represented by a tab in the interface

class Settings;
class SettingsEntry;
typedef std::unique_ptr<class SettingsSection> SettingsSectionPtr;

class SettingsCategory
{
public:

    SettingsCategory(Settings& settings, const QString& name, const QString& label);

    SettingsSection* getSection(const QString& name) const;
    SettingsEntry* getEntry(const QString& path) const;
    SettingsEntry* getEntry(const QStringList& pathSplitted) const;

    const QString& getName() const { return _name; }

protected:

    Settings& _settings;
    QString _name;
    std::vector<SettingsSectionPtr> sections;
};

#endif // SETTINGSCATEGORY_H
