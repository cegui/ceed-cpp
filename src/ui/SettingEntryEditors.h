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

protected:

    SettingsEntry& _entry;
};

class SettingSectionWidget : public QGroupBox
{
public:

    SettingSectionWidget(SettingsSection& section, QWidget* parent = nullptr);

protected:

    SettingsSection& _section;
};

class SettingCategoryWidget : public QScrollArea
{
public:

    SettingCategoryWidget(SettingsCategory& category, QWidget* parent = nullptr);

protected:

    SettingsCategory& _category;
};

#endif // SETTINGENTRYEDITORS_H
