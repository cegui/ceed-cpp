#ifndef SETTINGENTRYEDITORS_H
#define SETTINGENTRYEDITORS_H

#include "qboxlayout.h"
#include "qgroupbox.h"
#include "qscrollarea.h"

class SettingsEntry;
class SettingsSection;
class SettingsCategory;
class QLineEdit;

class SettingEntryEditorBase : public QHBoxLayout
{
public:

    SettingEntryEditorBase(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void discardChanges();

protected slots:

    virtual void resetToDefaultValue() = 0;

protected:

    void addResetButton();
    void updateUIOnChange();

    SettingsEntry& _entry;
};

class SettingEntryEditorString : public SettingEntryEditorBase
{
public:

    SettingEntryEditorString(SettingsEntry& entry, QWidget* parent = nullptr);

    virtual void discardChanges() override;

private slots:

    void onChange(const QString& text);

    QLineEdit* entryWidget = nullptr;
};

class SettingSectionWidget : public QGroupBox
{
public:

    SettingSectionWidget(SettingsSection& section, QWidget* parent = nullptr);

    void discardChanges();
    void onChange(SettingEntryEditorBase& entry);

protected:

    SettingsSection& _section;
    std::vector<SettingEntryEditorBase*> modifiedEntries;
};

class SettingCategoryWidget : public QScrollArea
{
public:

    SettingCategoryWidget(SettingsCategory& category, QWidget* parent = nullptr);

    void discardChanges();
    void onChange(SettingSectionWidget& section);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    void updateUIOnChange(bool deep);

    SettingsCategory& _category;
    std::vector<SettingSectionWidget*> modifiedSections;
};

#endif // SETTINGENTRYEDITORS_H
