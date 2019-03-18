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

void SettingEntryEditorBase::discardChanges()
{
    /*
        self.entry.hasChanges = False
    */
}

void SettingEntryEditorBase::addResetButton()
{
    auto btn = new QPushButton();
    btn->setIcon(QIcon(":/icons/settings/reset_entry_to_default.png"));
    btn->setIconSize(QSize(16, 16));
    btn->setToolTip("Reset this settings entry to the default value");
    addWidget(btn);

    //connect(btn, SIGNAL(clicked()), this, SLOT(resetToDefaultValue()));
    connect(btn, &QPushButton::clicked, this, &SettingEntryEditorBase::resetToDefaultValue);
}

void SettingEntryEditorBase::updateUIOnChange()
{
    SettingSectionWidget* parentWidget = static_cast<SettingSectionWidget*>(parent());
    parentWidget->onChange(*this);
    auto label = static_cast<QLabel*>(static_cast<QFormLayout*>(parentWidget->layout())->labelForField(this));
    label->setText(_entry.getLabel());
}

/*
    def markAsChanged(self):
        self.entry.markAsChanged()

    def markAsUnchanged(self):
        self.entry.markAsUnchanged()
*/

//---------------------------------------------------------------------

SettingEntryEditorString::SettingEntryEditorString(SettingsEntry& entry, QWidget* parent)
    : SettingEntryEditorBase(entry, parent)
{
    entryWidget = new QLineEdit();
    entryWidget->setText(entry.value().toString());
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorString::onChange);
}

void SettingEntryEditorString::discardChanges()
{
    entryWidget->setText(_entry.value().toString());
    SettingEntryEditorBase::discardChanges();
}

void SettingEntryEditorString::onChange(const QString& text)
{
    _entry.setEditedValue(text);
    updateUIOnChange();
}

void SettingEntryEditorString::resetToDefaultValue()
{
    if (_entry.editedValue() != _entry.defaultValue())
    {
        onChange(_entry.defaultValue().toString());
        entryWidget->setText(_entry.defaultValue().toString());
    }
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

        //if (entry->getWidgetHint() == "string")
            layout->addRow(label, new SettingEntryEditorString(*entry, this));
        //else
        //{
        //    // TODO: error message with a widget hint
        //    assert(false && "SettingSectionWidget::SettingSectionWidget() > unknown widget hint!");
        //}
        /*
    elif entry.widgetHint == "int":
        return InterfaceEntryInt(entry, parent)
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

void SettingSectionWidget::discardChanges()
{
    for (auto&& entry : modifiedEntries)
        entry->discardChanges();
}

void SettingSectionWidget::onChange(SettingEntryEditorBase& entry)
{
    modifiedEntries.push_back(&entry);

    SettingCategoryWidget* parentWidget = static_cast<SettingCategoryWidget*>(parent());
    parentWidget->onChange(*this);
}

/*
    def markAsUnchanged(self):
        self.section.markAsUnchanged()
        labelForField = self.layout.labelForField
        for entry in self.modifiedEntries:
            entry.markAsUnchanged()
            # FIXME: This should be rolled into the InterfaceEntry types.
            labelForField(entry).setText(entry.entry.label)
        self.modifiedEntries = []
*/

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

void SettingCategoryWidget::discardChanges()
{
    for (auto&& section : modifiedSections)
        section->discardChanges();
}

void SettingCategoryWidget::onChange(SettingSectionWidget& section)
{
    modifiedSections.push_back(&section);
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
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parent());
    tabs->setTabText(tabs->indexOf(this), _category.getLabel());
}
/*
    def markAsUnchanged(self):
        parent = self.parent
        self.category.markAsUnchanged()
        parent.setTabText(parent.indexOf(self), self.category.label)

        for section in self.modifiedSections:
            section.markAsUnchanged()

        self.modifiedSections = []
*/
