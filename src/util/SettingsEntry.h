#ifndef SETTINGSENTRY_H
#define SETTINGSENTRY_H

#include "qvariant.h"

// Is the value itself, inside a section. This is what's directly used when
// accessing settings.
// - value represents the current value to use
// - editedValue represents the value user directly edits
// (it is applied - value = editedValue - when user applies the settings)

class SettingsSection;

class SettingsEntry : public QObject
{
    Q_OBJECT

public:

    SettingsEntry(SettingsSection& section, const QString& name, const QVariant& defaultValue, const QString& label = QString(),
                  const QString& help = QString(), const QString& widgetHint = "string", bool changeRequiresRestart = false,
                  int sortingWeight = 0);
    virtual ~SettingsEntry() override;

    const QString& getName() const { return _name; }
    QString getLabel() const { return (_changed ? "* " : "") + _label; }
    QString getPath() const;
    QString getHelp() const { return _help; }
    QString getWidgetHint() const { return _widgetHint; }
    SettingsSection& getSection() const { return _section; }
    int getSortingWeight() const { return _sortingWeight; }

    void setValue(const QVariant& val, bool storeImmediately = true);
    void setEditedValue(const QVariant& val);
    QVariant& value() { return _value; }
    QVariant& editedValue() { return _editedValue; }
    const QVariant& defaultValue() const { return _defaultValue; }
    bool isModified() const { return _changed; } //???_editedValue != _value instead?

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

    int _sortingWeight = 0;
    bool _changeRequiresRestart = false;
    bool _changed = false;
};

#endif // SETTINGSENTRY_H
