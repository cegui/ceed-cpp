#ifndef SETTINGS_H
#define SETTINGS_H

#include "qstring.h"
#include "vector"

// Application global settings system

class QSettings;
class SettingsEntry;
typedef std::unique_ptr<class SettingsCategory> SettingsCategoryPtr;

class Settings
{
public:

    Settings(QSettings* qsettings);
    ~Settings();

    SettingsCategory* createCategory(const QString& name, const QString& label);
    SettingsCategory* getCategory(const QString& name) const;
    SettingsEntry* getEntry(const QString& path) const;
    SettingsEntry* getEntry(QStringList pathSplitted) const;

    QString getPath() const { return "settings"; }
    QSettings* getQSettings() const { return _qsettings; }

    void applyChanges();
    void discardChanges();
    void load();
    void store();

    void sort(bool deep = true);

    void markRequiresRestart() { _changesRequireRestart = true; }

protected:

    QSettings* _qsettings = nullptr;
    std::vector<SettingsCategoryPtr> categories;
    bool _changesRequireRestart = false;
};

#endif // SETTINGS_H
