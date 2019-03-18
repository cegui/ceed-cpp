#include "src/ui/SettingEntryEditors.h"
#include "src/util/SettingsEntry.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsCategory.h"
#include "qformlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qevent.h"
#include "qscrollbar.h"
#include "qtabwidget.h"
#include "qvalidator.h"

// Implementation notes
// - The "change detection" scheme propagates upwards from the individual Entry
//   types to their parents (currently terminated at the Category/Tab level).
// - In contrast, when a user applies changes, this propagates downwards from
//   the Category/Tab level to the individual (modified) entries.
// - The reason is because the settings widgets (QLineEdit, QCheckBox, etc) are
//   used to notify the application when a change happens; and once changes are
//   applied, it is convenient to use an iterate/apply mechanism.

SettingEntryEditorBase::SettingEntryEditorBase(SettingsEntry& entry, QWidget* parent)
    : QHBoxLayout(parent)
    , _entry(entry)
{
}

void SettingEntryEditorBase::addResetButton()
{
    auto btn = new QPushButton();
    btn->setIcon(QIcon(":/icons/settings/reset_entry_to_default.png"));
    btn->setIconSize(QSize(16, 16));
    btn->setToolTip("Reset this settings entry to the default value");
    addWidget(btn);

    connect(btn, &QPushButton::clicked, this, &SettingEntryEditorBase::resetToDefaultValue);
}

void SettingEntryEditorBase::updateUIOnChange()
{
    // QFormLayout -> QGroupBox(SettingSectionWidget)
    SettingSectionWidget* parentWidget = static_cast<SettingSectionWidget*>(parent()->parent());
    parentWidget->onChange();
    auto label = static_cast<QLabel*>(static_cast<QFormLayout*>(parentWidget->layout())->labelForField(this));
    label->setText(_entry.getLabel());
}

void SettingEntryEditorBase::resetToDefaultValue()
{
    if (_entry.editedValue() != _entry.defaultValue())
    {
        _entry.setEditedValue(_entry.defaultValue());
        updateUIOnChange();
        updateValueInUI();
    }
}

//---------------------------------------------------------------------

SettingEntryEditorString::SettingEntryEditorString(SettingsEntry& entry, QWidget* parent)
    : SettingEntryEditorBase(entry, parent)
{
    assert(entry.defaultValue().canConvert(QVariant::String));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorString::onChange);

    updateValueInUI();
}

void SettingEntryEditorString::updateValueInUI()
{
    entryWidget->setText(_entry.editedValue().toString());
}

void SettingEntryEditorString::onChange(const QString& text)
{
    _entry.setEditedValue(text);
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorInt::SettingEntryEditorInt(SettingsEntry& entry, QWidget* parent)
    : SettingEntryEditorBase(entry, parent)
{
    assert(entry.defaultValue().canConvert(QVariant::Int));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    entryWidget->setValidator(new QIntValidator(0, std::numeric_limits<int>().max(), this)); // TODO: limits from setting entry!
    addWidget(entryWidget, 1);
    addResetButton();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorInt::onChange);

    updateValueInUI();
}

void SettingEntryEditorInt::updateValueInUI()
{
    entryWidget->setText(_entry.editedValue().toString());
}

//???catch only Enter / focus lost 'end editing'?
void SettingEntryEditorInt::onChange(const QString& text)
{
    if (text.isEmpty())
    {
        _entry.setEditedValue(0);
        //updateValueInUI();
    }
    else
    {
        bool converted = false;
        const int val = text.toInt(&converted);
        assert(converted);
        _entry.setEditedValue(val);
    }
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorFloat::SettingEntryEditorFloat(SettingsEntry& entry, QWidget* parent)
    : SettingEntryEditorBase(entry, parent)
{
    assert(entry.defaultValue().canConvert(QVariant::Double));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    entryWidget->setValidator(new QDoubleValidator(this)); // TODO: limits from setting entry!
    entryWidget->setText(entry.value().toString());
    addWidget(entryWidget, 1);
    addResetButton();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorFloat::onChange);
}

void SettingEntryEditorFloat::updateValueInUI()
{
    entryWidget->setText(_entry.editedValue().toString());
}

//???catch only Enter / focus lost 'end editing'?
void SettingEntryEditorFloat::onChange(const QString& text)
{
    if (text.isEmpty())
    {
        _entry.setEditedValue(0.f);
        //updateValueInUI();
    }
    else
    {
        bool converted = false;
        const float val = text.toFloat(&converted);
        assert(converted);
        _entry.setEditedValue(val);
    }
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingSectionWidget::SettingSectionWidget(SettingsSection& section, QWidget* parent)
    : QGroupBox(parent)
    , _section(section)
{
    setTitle(section.getLabel());

    // NB: 'this' as a parent is required as of Qt 5.12.1!
    // Else layout will not show ALL except the first SettingEntryEditorString,
    // and if QLabel below will have 'this' parent it will be rendered buggy.
    auto layout = new QFormLayout(this);

    for (auto&& entry : section.getEntries())
    {
        auto label = new QLabel(entry->getLabel());
        label->setMinimumWidth(200);
        label->setWordWrap(true);

        //???for empty hint check value type, option list etc?

        if (entry->getWidgetHint() == "int")
            layout->addRow(label, new SettingEntryEditorInt(*entry, this));
        else if (entry->getWidgetHint() == "string")
            layout->addRow(label, new SettingEntryEditorString(*entry, this));
        else
            layout->addRow(label, new QLabel("Unknown widget hint: " + entry->getWidgetHint()));
        /*
    elif entry.widgetHint == "float":
        return InterfaceEntryFloat(entry, parent)
    elif entry.widgetHint == "checkbox":
        return InterfaceEntryCheckbox(entry, parent)
    elif entry.widgetHint == "colour":
        return InterfaceEntryColour(entry, parent)
    elif entry.widgetHint == "pen":
        return InterfaceEntryPen(entry, parent)
    elif entry.widgetHint == "keySequence":
        return InterfaceEntryKeySequence(entry, parent)
    elif entry.widgetHint == "combobox":
        return InterfaceEntryCombobox(entry, parent)
        */
    }

    setLayout(layout);
}

void SettingSectionWidget::updateValuesInUI()
{    
    for (int i = 0; i < layout()->count(); ++i)
    {
        auto childLayout = layout()->itemAt(i)->layout();
        if (childLayout)
            static_cast<SettingEntryEditorBase*>(childLayout)->updateValueInUI();
    }
}

void SettingSectionWidget::onChange()
{
    // QVBoxLayout -> QWidget inner -> QScrollArea(SettingCategoryWidget)
    SettingCategoryWidget* parentWidget = static_cast<SettingCategoryWidget*>(parent()->parent()->parent());
    parentWidget->onChange();
}

void SettingSectionWidget::updateUIOnChange(bool deep)
{
    // Update 'modified' mark (has no one for now)

    if (!deep) return;

    for (int i = 0; i < layout()->count(); ++i)
    {
        auto childLayout = layout()->itemAt(i)->layout();
        if (childLayout)
            static_cast<SettingEntryEditorBase*>(childLayout)->updateUIOnChange();
    }
}

//---------------------------------------------------------------------

SettingCategoryWidget::SettingCategoryWidget(SettingsCategory& category, QWidget* parent)
    : QScrollArea(parent)
    , _category(category)
{
    auto inner = new QWidget();
    auto layout = new QVBoxLayout();

    for (auto&& section : category.getSections())
        layout->addWidget(new SettingSectionWidget(*section, this));

    layout->addStretch();
    inner->setLayout(layout);
    setWidget(inner);
    setWidgetResizable(true);
}

void SettingCategoryWidget::updateValuesInUI()
{
   QVBoxLayout* myLayout = static_cast<QVBoxLayout*>(widget()->layout());
   for (int i = 0; i < myLayout->count(); ++i)
   {
       auto childWidget = myLayout->itemAt(i)->widget();
       if (childWidget)
           static_cast<SettingSectionWidget*>(childWidget)->updateValuesInUI();
   }
}

void SettingCategoryWidget::onChange()
{
    updateUIOnChange(false);
}

bool SettingCategoryWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Wheel)
    {
        if (static_cast<QWheelEvent*>(event)->delta() < 0)
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        else
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        return true;
    }
    return QScrollArea::eventFilter(watched, event);
}

void SettingCategoryWidget::updateUIOnChange(bool deep)
{
    // Layout -> QTabWidget of the SettingsDialog
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parent()->parent());
    tabs->setTabText(tabs->indexOf(this), _category.getLabel());

    if (!deep) return;

    QVBoxLayout* myLayout = static_cast<QVBoxLayout*>(widget()->layout());
    for (int i = 0; i < myLayout->count(); ++i)
    {
        auto childWidget = myLayout->itemAt(i)->widget();
        if (childWidget)
            static_cast<SettingSectionWidget*>(childWidget)->updateUIOnChange(true);
    }
}
