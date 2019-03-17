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

/*
    def discardChanges(self):
        self.entry.hasChanges = False

    def onChange(self, _):
        self.markAsChanged()
        self.parent.onChange(self)

    def markAsChanged(self):
        self.entry.markAsChanged()

    def markAsUnchanged(self):
        self.entry.markAsUnchanged()
*/

//---------------------------------------------------------------------

SettingEntryEditorString::SettingEntryEditorString(SettingsEntry& entry, QWidget* parent)
    : SettingEntryEditorBase(entry, parent)
{
    auto entryWidget = new QLineEdit();
    entryWidget->setText(entry.value().toString());
    entryWidget->setToolTip(entry.getHelp());
    addWidget(entryWidget, 1);
    addResetButton();

    connect(entryWidget, &QLineEdit::textEdited, this, &SettingEntryEditorString::onChange);
}

void SettingEntryEditorString::onChange(const QString& text)
{
    _entry.setEditedValue(text);

    /*
    super(InterfaceEntryString, self).onChange(text)
    */

    /*
        # FIXME: This should be rolled into the InterfaceEntry types.
        parent.layout.labelForField(entry).setText(entry.entry.label)
    */
    SettingSectionWidget* parentWidget = qobject_cast<SettingSectionWidget*>(parent());
    parentWidget->onChange(_entry);
    //parentWidget->layout()->labe
}

/*
    def discardChanges(self):
        self.entryWidget.setText(str(self.entry.value))
        super(InterfaceEntryString, self).discardChanges()
*/

//---------------------------------------------------------------------

SettingSectionWidget::SettingSectionWidget(SettingsSection& section, QWidget* parent)
    : QGroupBox(parent)
    , _section(section)
{
    setTitle(section.getLabel());

    auto layout = new QFormLayout();
/*
    self.modifiedEntries = []
*/

    for (auto&& entry : section.getEntries())
    {
        auto label = new QLabel(entry->getLabel());
        label->setMinimumWidth(200);
        label->setWordWrap(true);

        if (entry->getWidgetHint() == "string")
            layout->addRow(label, new SettingEntryEditorString(*entry, this));
        else
        {
            // TODO: error message with a widget hint
            assert(false && "SettingSectionWidget::SettingSectionWidget() > unknown widget hint!");
        }
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

void SettingSectionWidget::onChange(SettingsEntry& entry)
{
    modifiedEntries.push_back(&entry);

    SettingCategoryWidget* parentWidget = qobject_cast<SettingCategoryWidget*>(parent());
    parentWidget->onChange(_section);
}

/*
class InterfaceSection(QtGui.QGroupBox):

    def discardChanges(self):
        for entry in self.modifiedEntries:
            entry.discardChanges()

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

void SettingCategoryWidget::onChange(SettingsSection& section)
{
    modifiedSections.push_back(&section);

    //!!!updateUI(bool deep)!
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parent());
    tabs->setTabText(tabs->indexOf(this), _category.getLabel());
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
/*
    def discardChanges(self):
        for section in self.modifiedSections:
            section.discardChanges()

    def markAsUnchanged(self):
        parent = self.parent
        self.category.markAsUnchanged()
        parent.setTabText(parent.indexOf(self), self.category.label)

        for section in self.modifiedSections:
            section.markAsUnchanged()

        self.modifiedSections = []
*/
