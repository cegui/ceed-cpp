#include "src/ui/SettingEntryEditors.h"
#include "src/util/SettingsEntry.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsCategory.h"
#include "qformlayout.h"
#include "qlabel.h"

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

/*
    def _addBasicWidgets(self):
        self.addWidget(self.entryWidget, 1)
        self.addWidget(self._buildResetButton())

    def _buildResetButton(self):
        self.entryWidget.slot_resetToDefault = self.resetToDefaultValue
        ret = QtGui.QPushButton()
        ret.setIcon(QtGui.QIcon("icons/settings/reset_entry_to_default.png"))
        ret.setIconSize(QtCore.QSize(16, 16))
        ret.setToolTip("Reset this settings entry to the default value")
        ret.clicked.connect(self.entryWidget.slot_resetToDefault)
        return ret

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

        /*
        layout->addRow(label, interfaceEntryFactory(entry, self))
        */
    }

    setLayout(layout);
}

/*
class InterfaceSection(QtGui.QGroupBox):

    def discardChanges(self):
        for entry in self.modifiedEntries:
            entry.discardChanges()

    def onChange(self, entry):
        self.modifiedEntries.append(entry)
        self.markAsChanged()
        # FIXME: This should be rolled into the InterfaceEntry types.
        self.layout.labelForField(entry).setText(entry.entry.label)
        self.parent.onChange(self)

    def markAsChanged(self):
        self.section.markAsChanged()

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

/*
        self.modifiedSections = []
*/
    for (auto&& section : category.getSections())
        layout->addWidget(new SettingSectionWidget(*section, this));

    layout->addStretch();
    inner->setLayout(layout);
    setWidget(inner);
    setWidgetResizable(true);
}

/*
# Wrapper: Category
class InterfaceCategory(QtGui.QScrollArea):

    def eventFilter(self, obj, event):
        if event.type() == QtCore.QEvent.Wheel:
            if event.delta() < 0:
                self.verticalScrollBar().triggerAction(QtGui.QAbstractSlider.SliderSingleStepAdd)
            else:
                self.verticalScrollBar().triggerAction(QtGui.QAbstractSlider.SliderSingleStepSub)
            return True

        return super(InterfaceCategory, self).eventFilter(obj, event)

    def discardChanges(self):
        for section in self.modifiedSections:
            section.discardChanges()

    def onChange(self, section):
        self.modifiedSections.append(section)
        self.markAsChanged()

    def markAsChanged(self):
        parent = self.parent
        self.category.markAsChanged()
        parent.setTabText(parent.indexOf(self), self.category.label)

    def markAsUnchanged(self):
        parent = self.parent
        self.category.markAsUnchanged()
        parent.setTabText(parent.indexOf(self), self.category.label)

        for section in self.modifiedSections:
            section.markAsUnchanged()

        self.modifiedSections = []
*/
