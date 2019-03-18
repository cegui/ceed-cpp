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

    virtual void discardChangesInUI() = 0;
    void updateUIOnChange();

protected slots:

    virtual void resetToDefaultValue() = 0;

protected:

    void addResetButton();

    SettingsEntry& _entry;
};

//---------------------------------------------------------------------

class SettingEntryEditorString : public SettingEntryEditorBase
{
public:

    SettingEntryEditorString(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void discardChangesInUI() override;

private slots:

    void onChange(const QString& text);
    virtual void resetToDefaultValue() override;

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorInt : public SettingEntryEditorBase
{
public:

    SettingEntryEditorInt(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void discardChangesInUI() override;

private slots:

    void onChange(const QString& text);
    virtual void resetToDefaultValue() override;

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingSectionWidget : public QGroupBox
{
public:

    SettingSectionWidget(SettingsSection& section, QWidget* parent = nullptr);

    void discardChangesInUI();
    void onChange(SettingEntryEditorBase& entry);
    void updateUIOnChange(bool deep);

protected:

    SettingsSection& _section;
    std::set<SettingEntryEditorBase*> modifiedEntries;
};

//---------------------------------------------------------------------

class SettingCategoryWidget : public QScrollArea
{
public:

    SettingCategoryWidget(SettingsCategory& category, QWidget* parent = nullptr);

    void discardChangesInUI();
    void onChange(SettingSectionWidget& section);
    void updateUIOnChange(bool deep);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    SettingsCategory& _category;
    std::set<SettingSectionWidget*> modifiedSections;
};

#endif // SETTINGENTRYEDITORS_H
