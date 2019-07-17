#include "src/ui/SettingEntryEditors.h"
#include "src/util/SettingsEntry.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsCategory.h"
#include "src/ui/widgets/ColourButton.h"
#include "src/ui/widgets/PenButton.h"
#include "src/ui/widgets/KeySequenceButton.h"
#include "qformlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qevent.h"
#include "qscrollbar.h"
#include "qtabwidget.h"
#include "qvalidator.h"
#include "qcheckbox.h"
#include "qcombobox.h"

// Implementation notes
// - The "change detection" scheme propagates upwards from the individual Entry
//   types to their parents (currently terminated at the Category/Tab level).
// - In contrast, when a user applies changes, this propagates downwards from
//   the Category/Tab level to the individual (modified) entries.
// - The reason is because the settings widgets (QLineEdit, QCheckBox, etc) are
//   used to notify the application when a change happens; and once changes are
//   applied, it is convenient to use an iterate/apply mechanism.

SettingEntryEditorBase::SettingEntryEditorBase(SettingsEntry& entry)
    : QHBoxLayout()
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

SettingEntryEditorString::SettingEntryEditorString(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::String));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorString::onChange);
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

SettingEntryEditorInt::SettingEntryEditorInt(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::Int));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    entryWidget->setValidator(new QIntValidator(0, std::numeric_limits<int>().max(), this)); // TODO: limits from setting entry!
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorInt::onChange);
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

SettingEntryEditorFloat::SettingEntryEditorFloat(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::Double));

    entryWidget = new QLineEdit();
    entryWidget->setToolTip(entry.getHelp());
    entryWidget->setValidator(new QDoubleValidator(this)); // TODO: limits from setting entry!
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

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

SettingEntryEditorCheckbox::SettingEntryEditorCheckbox(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::Bool));

    entryWidget = new QCheckBox();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &QCheckBox::stateChanged, this, &SettingEntryEditorCheckbox::onChange);
}

void SettingEntryEditorCheckbox::updateValueInUI()
{
    entryWidget->setChecked(_entry.editedValue().toBool());
}

void SettingEntryEditorCheckbox::onChange(bool state)
{
    _entry.setEditedValue(state);
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorColour::SettingEntryEditorColour(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::Color));

    entryWidget = new ColourButton();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &ColourButton::colourChanged, this, &SettingEntryEditorColour::onChange);
}

void SettingEntryEditorColour::updateValueInUI()
{
    entryWidget->setColour(_entry.editedValue().value<QColor>());
}

void SettingEntryEditorColour::onChange(const QColor& colour)
{
    _entry.setEditedValue(colour);
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorPen::SettingEntryEditorPen(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::Pen));

    entryWidget = new PenButton();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &PenButton::penChanged, this, &SettingEntryEditorPen::onChange);
}

void SettingEntryEditorPen::updateValueInUI()
{
    entryWidget->setPen(_entry.editedValue().value<QPen>());
}

void SettingEntryEditorPen::onChange(const QPen& pen)
{
    _entry.setEditedValue(pen);
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorKeySequence::SettingEntryEditorKeySequence(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(entry.defaultValue().canConvert(QVariant::KeySequence));

    entryWidget = new KeySequenceButton();
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, &KeySequenceButton::keySequenceChanged, this, &SettingEntryEditorKeySequence::onChange);
}

void SettingEntryEditorKeySequence::updateValueInUI()
{
    entryWidget->setKeySequence(_entry.editedValue().value<QKeySequence>());
}

void SettingEntryEditorKeySequence::onChange(const QKeySequence& seq)
{
    _entry.setEditedValue(seq);
    updateUIOnChange();
}

//---------------------------------------------------------------------

SettingEntryEditorCombobox::SettingEntryEditorCombobox(SettingsEntry& entry)
    : SettingEntryEditorBase(entry)
{
    assert(!entry.getOptionList().empty());

    entryWidget = new QComboBox();

    for (auto&& option : entry.getOptionList())
        entryWidget->addItem(option.second, option.first);

    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    updateValueInUI();

    connect(entryWidget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingEntryEditorCombobox::onChange);
}

void SettingEntryEditorCombobox::updateValueInUI()
{
    int index = entryWidget->findData(_entry.editedValue());
    if (index >= 0)
        entryWidget->setCurrentIndex(index);
}

void SettingEntryEditorCombobox::onChange(int index)
{
    if (index >= 0)
    {
        _entry.setEditedValue(entryWidget->itemData(index));
        updateUIOnChange();
    }
}

//---------------------------------------------------------------------

SettingSectionWidget::SettingSectionWidget(SettingsSection& section, QWidget* parent)
    : QGroupBox(parent)
    , _section(section)
{
    setTitle(section.getLabel());

    auto newLayout = new QFormLayout(this);

    for (auto&& entry : section.getEntries())
    {
        auto label = new QLabel(entry->getLabel());
        label->setMinimumWidth(200);
        label->setWordWrap(true);
        label->setToolTip(entry->getHelp());

        if (entry->getWidgetHint() == "string")
            newLayout->addRow(label, new SettingEntryEditorString(*entry));
        else if (entry->getWidgetHint() == "int")
            newLayout->addRow(label, new SettingEntryEditorInt(*entry));
        else if (entry->getWidgetHint() == "float")
            newLayout->addRow(label, new SettingEntryEditorFloat(*entry));
        else if (entry->getWidgetHint() == "checkbox")
            newLayout->addRow(label, new SettingEntryEditorCheckbox(*entry));
        else if (entry->getWidgetHint() == "colour")
            newLayout->addRow(label, new SettingEntryEditorColour(*entry));
        else if (entry->getWidgetHint() == "pen")
            newLayout->addRow(label, new SettingEntryEditorPen(*entry));
        else if (entry->getWidgetHint() == "keySequence")
            newLayout->addRow(label, new SettingEntryEditorKeySequence(*entry));
        else if (entry->getWidgetHint() == "combobox")
            newLayout->addRow(label, new SettingEntryEditorCombobox(*entry));
        else
            newLayout->addRow(label, new QLabel("Unknown widget hint: " + entry->getWidgetHint()));
    }
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
    auto newLayout = new QVBoxLayout();

    for (auto&& section : category.getSections())
        newLayout->addWidget(new SettingSectionWidget(*section, this));

    newLayout->addStretch();
    inner->setLayout(newLayout);
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
