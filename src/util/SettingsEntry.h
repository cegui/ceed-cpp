#ifndef SETTINGSENTRY_H
#define SETTINGSENTRY_H

#include "qvariant.h"

// Is the value itself, inside a section. This is what's directly used when
// accessing settings.
// - value represents the current value to use
// - editedValue represents the value user directly edits
// (it is applied - value = editedValue - when user applies the settings)

class SettingsSection;

class SettingsEntry
{
public:

    SettingsEntry(SettingsSection& section, const QString& name, const QVariant& defaultValue, const QString& label = QString(),
                  const QString& help = QString(), const QString& widgetHint = "string", bool changeRequiresRestart = false);
    ~SettingsEntry();

    const QString& getName() const { return _name; }
    QString getLabel() const { return (_changed ? "* " : "") + _label; }
    QString getPath() const;

    void setValue(const QVariant& val, bool storeImmediately = true);
    void setEditedValue(const QVariant& val);
    QVariant& value() { return _value; }
    QVariant& editedValue() { return _editedValue; }

    void applyChanges();
    void discardChanges();
    void load();
    void store();

signals:

    void valueChanged(const QVariant& newValue);

protected:

    bool sanitizeValue(QVariant& val) const;

    SettingsSection& _section;
    QString _name;
    QString _label;
    QString _help;
    QString _widgetHint;

    QVariant _value;
    QVariant _editedValue;
    QVariant _defaultValue;

    bool _changeRequiresRestart = false;
    bool _changed = false;
};

#endif // SETTINGSENTRY_H
