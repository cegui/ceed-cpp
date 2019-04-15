#include "src/ui/PropertyInspectorWidget.h"
#include "src/ui/widgets/LineEditWithClearButton.h"
#include "qboxlayout.h"
#include "qlabel.h"

static const QString modifiedFilterPrefix = "*";

// Full blown inspector widget for CEGUI PropertySet(s). Requires a call to 'setPropertyManager()'
// before it can show properties via 'setPropertySets'.
PropertyInspectorWidget::PropertyInspectorWidget(QWidget *parent)
    : QWidget(parent)
    , _selectionObjectDescription("Nothing is selected.")
{
    auto myLayout = new QVBoxLayout();
    myLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(myLayout);

    // If the filter starts with this, we only show modified properties
    auto filterBox = new LineEditWithClearButton();
    filterBox->setPlaceholderText(QString("Filter (prefix with '%q' to show modified)").arg(modifiedFilterPrefix));
    connect(filterBox, &LineEditWithClearButton::textChanged, this, &PropertyInspectorWidget::filterChanged);

    _selectionLabel = new QLabel();
    _selectionLabel->setAlignment(Qt::AlignCenter);
    _selectionLabel->setFrameStyle(QFrame::StyledPanel);
    _selectionLabel->setFrameShadow(QFrame::Sunken);

/*
        self.ptree = .propertytree.ui.PropertyTreeWidget()

        self.propertyManager = None

        self.currentSource = []
*/

    myLayout->addWidget(_selectionLabel);
    myLayout->addWidget(filterBox);
/*
    myLayout.addWidget(self.ptree)
*/

    // Set the minimum size to a reasonable value for this widget
    setMinimumSize(200, 200);
}

void PropertyInspectorWidget::filterChanged(const QString& filterText)
{
/*
        if filterText and filterText.startswith(self.modifiedFilterPrefix):
            self.ptree.setFilter(filterText[len(self.modifiedFilterPrefix):], True)
        else:
            self.ptree.setFilter(filterText)
*/
}

void PropertyInspectorWidget::resizeEvent(QResizeEvent* event)
{
/*
    def resizeEvent(self, QResizeEvent):
        self.updateSelectionLabelElidedText()

        super(PropertyInspectorWidget, self).resizeEvent(QResizeEvent)
*/
}

// Shortens the window/widget path so that the whole text will fit into the label.
// The beginning of the, if necessary, cut-off path text will be "...".
void PropertyInspectorWidget::updateSelectionLabelElidedText()
{
    QString adjustedSelectionObjectPath;
    if (!_selectionObjectPath.isEmpty())
        adjustedSelectionObjectPath = _selectionObjectPath + " : ";

    auto fontMetrics = _selectionLabel->fontMetrics();
    int labelWidth = _selectionLabel->size().width();
    int objectDescriptionWidth = fontMetrics.width(_selectionObjectDescription);
    int objectPathWidth = fontMetrics.width(adjustedSelectionObjectPath);
    int margin = 6;
    int minWidthTakenByPath = 20;

    QString finalText;
    if (labelWidth > objectDescriptionWidth + objectPathWidth)
        finalText = adjustedSelectionObjectPath + _selectionObjectDescription;
    else if (labelWidth < minWidthTakenByPath + objectDescriptionWidth)
        finalText = fontMetrics.elidedText(_selectionObjectDescription, Qt::ElideRight, labelWidth - margin);
    else
    {
        auto alteredPathText = fontMetrics.elidedText(adjustedSelectionObjectPath, Qt::ElideLeft, labelWidth - margin - objectDescriptionWidth);
        finalText = alteredPathText + _selectionObjectDescription;
    }

    _selectionLabel->setText(finalText);
    _selectionLabel->setToolTip(_selectionLabelTooltip);
}

/*
    def setPropertyManager(self, propertyManager):
        self.propertyManager = propertyManager

    @staticmethod
    def generateLabelForSet(ceguiPropertySet):
        # We do not know what the property set is but we can take a few informed
        # guesses. Most likely it will be a CEGUI::Window.

        if isinstance(ceguiPropertySet, PyCEGUI.Window):
            return ceguiPropertySet.getNamePath(), ceguiPropertySet.getType()
        else:
            return "", "Unknown PropertySet"

    def setSource(self, source):

        #We check what kind of source we are dealing with
        if type(source) is list:
            if len(source) == 0:
                self.selectionObjectPath = ""
                self.selectionObjectDescription = "Nothing is selected."
                self.selectionLabelTooltip = ""

            elif len(source) == 1:
                self.selectionObjectPath, self.selectionObjectDescription = PropertyInspectorWidget.generateLabelForSet(source[0])

                selectionInfoTuple = (self.selectionObjectPath, self.selectionObjectDescription)
                self.selectionLabelTooltip = " : ".join(selectionInfoTuple)

            else:
                tooltip = ""
                for ceguiPropertySet in source:
                    path, typeName = PropertyInspectorWidget.generateLabelForSet(ceguiPropertySet)

                    selectionInfoTuple = (path, typeName)
                    joinedPathAndName = " : ".join(selectionInfoTuple)
                    tooltip += joinedPathAndName + "\n"

                self.selectionObjectPath = ""
                self.selectionObjectDescription = "Multiple selections..."
                self.selectionLabelTooltip = tooltip.rstrip('\n')

        else:
            #Otherwise it must be a FalagardElement
            from ceed.editors.looknfeel.hierarchy_tree_item import LookNFeelHierarchyItem
            falagardEleName, falagardEleTooltip = LookNFeelHierarchyItem.getNameAndToolTip(source, "")
            self.selectionObjectPath = ""
            self.selectionObjectDescription = falagardEleName
            self.selectionLabelTooltip = falagardEleTooltip

        self.updateSelectionLabelElidedText()

        categories = self.propertyManager.buildCategories(source)

        # load them into the tree
        self.ptree.load(categories)

        self.currentSource = source

    def getSources(self):
        return self.currentSource
*/
