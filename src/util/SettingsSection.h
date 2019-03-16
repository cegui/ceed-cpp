#ifndef SETTINGSSECTION_H
#define SETTINGSSECTION_H

#include "qstring.h"
#include "vector"

// Groups entries, is usually represented by a group box in the interface

typedef std::unique_ptr<class SettingsEntry> SettingsEntryPtr;

class SettingsSection
{
public:

    SettingsSection();

    SettingsEntry* getEntry(const QString& name) const;

    const QString& getName() const { return _name; }

protected:

    std::vector<SettingsEntryPtr> entries;
    QString _name;
};

#endif // SETTINGSSECTION_H
