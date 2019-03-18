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
class QCheckBox;
class ColourButton;
class PenButton;

class SettingEntryEditorBase : public QHBoxLayout
{
public:

    SettingEntryEditorBase(SettingsEntry& entry);

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

    SettingEntryEditorString(SettingsEntry& entry);

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

    SettingEntryEditorInt(SettingsEntry& entry);

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

    SettingEntryEditorFloat(SettingsEntry& entry);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QString& text);

private:

    QLineEdit* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorCheckbox : public SettingEntryEditorBase
{
public:

    SettingEntryEditorCheckbox(SettingsEntry& entry);

    virtual void updateValueInUI() override;

private slots:

    void onChange(bool state);

private:

    QCheckBox* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorColour : public SettingEntryEditorBase
{
public:

    SettingEntryEditorColour(SettingsEntry& entry);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QColor& colour);

private:

    ColourButton* entryWidget = nullptr;
};

//---------------------------------------------------------------------

class SettingEntryEditorPen : public SettingEntryEditorBase
{
public:

    SettingEntryEditorPen(SettingsEntry& entry);

    virtual void updateValueInUI() override;

private slots:

    void onChange(const QPen& pen);

private:

    PenButton* entryWidget = nullptr;
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
