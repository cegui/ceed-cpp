#ifndef SETTINGENTRYEDITORS_H
#define SETTINGENTRYEDITORS_H

#include "qboxlayout.h"
#include "qgroupbox.h"
#include "qscrollarea.h"

class SettingsEntry;
class SettingsSection;
class SettingsCategory;

class SettingEntryEditorBase : public QHBoxLayout
{
public:

    SettingEntryEditorBase(SettingsEntry& entry, QWidget* parent = nullptr);

protected slots:

    virtual void resetToDefaultValue();

protected:

    void addResetButton();

    SettingsEntry& _entry;
};

class SettingEntryEditorString : public SettingEntryEditorBase
{
public:

    SettingEntryEditorString(SettingsEntry& entry, QWidget* parent = nullptr);

private slots:

    void onChange(const QString& text);
};

class SettingSectionWidget : public QGroupBox
{
public:

    SettingSectionWidget(SettingsSection& section, QWidget* parent = nullptr);

    void onChange(SettingsEntry& entry);

protected:

    SettingsSection& _section;
    std::vector<SettingsEntry*> modifiedEntries;
};

class SettingCategoryWidget : public QScrollArea
{
public:

    SettingCategoryWidget(SettingsCategory& category, QWidget* parent = nullptr);

    void onChange(SettingsSection& section);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    SettingsCategory& _category;
    std::vector<SettingsSection*> modifiedSections;
};

#endif // SETTINGENTRYEDITORS_H
