#ifndef SETTINGSCATEGORY_H
#define SETTINGSCATEGORY_H

#include "qstring.h"

// Groups sections, is usually represented by a tab in the interface

class Settings;

class SettingsCategory
{
public:

    SettingsCategory(Settings& settings, const QString& name, const QString& label);

    const QString& getName() const { return _name; }

protected:

    Settings& _settings;
    QString _name;
};

#endif // SETTINGSCATEGORY_H
