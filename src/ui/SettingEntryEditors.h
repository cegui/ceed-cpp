#ifndef SETTINGENTRYEDITORS_H
#define SETTINGENTRYEDITORS_H

#include "qboxlayout.h"
#include "qgroupbox.h"
#include "qscrollarea.h"
#include <set>

class SettingsEntry;
class SettingsSection;
class SettingsCategory;
class QLineEdit;

class SettingEntryEditorBase : public QHBoxLayout
{
public:

    SettingEntryEditorBase(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void updateValueInUI() = 0;
    void updateUIOnChange();

protected slots:

    void resetToDefaultValue();

protected:

    void addResetButton();

    SettingsEntry& _entry;
};

//---------------------------------------------------------------------

class SettingEntryEditorString : public SettingEntryEditorBase
{
public:

    SettingEntryEditorString(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QString& text);

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorInt : public SettingEntryEditorBase
{
public:

    SettingEntryEditorInt(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QString& text);

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorFloat : public SettingEntryEditorBase
{
public:

    SettingEntryEditorFloat(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QString& text);

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingSectionWidget : public QGroupBox
{
public:

    SettingSectionWidget(SettingsSection& section, QWidget* parent = nullptr);

    void updateValuesInUI();
    void onChange();
    void updateUIOnChange(bool deep);

protected:

    SettingsSection& _section;
};

//---------------------------------------------------------------------

class SettingCategoryWidget : public QScrollArea
{
public:

    SettingCategoryWidget(SettingsCategory& category, QWidget* parent = nullptr);

    void updateValuesInUI();
    void onChange();
    void updateUIOnChange(bool deep);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    SettingsCategory& _category;
};

#endif // SETTINGENTRYEDITORS_H