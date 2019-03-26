#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"
#include "ui_ImagesetEditorDockWidget.h"
#include "qitemdelegate.h"
#include "qvalidator.h"

// The only reason for this is to track when we are editing.
// We need this to discard key events when editor is open.
// TODO: Isn't there a better way to do this?
class ImageEntryItemDelegate : public QItemDelegate
{
public:

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        editing = true;
        QItemDelegate::setEditorData(editor, index);
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QItemDelegate::setModelData(editor, model, index);
        editing = false;
    }

protected:

      mutable bool editing = false;
};

ImagesetEditorDockWidget::ImagesetEditorDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ImagesetEditorDockWidget)
{
    ui->setupUi(this);

    /*
        self.visual = visual
    */

    ui->image->setInitialDirectoryDelegate([]()
    {
        auto project = CEGUIProjectManager::Instance().getCurrentProject();
        return project ? project->getResourceFilePath("", "imagesets") : "";
    });

    auto intValidator = new QIntValidator(0, 9999999, this);
    ui->nativeHorzRes->setValidator(intValidator);
    ui->nativeVertRes->setValidator(intValidator);
    connect(ui->nativeHorzRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionEdited);
    connect(ui->nativeVertRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionEdited);

    ui->list->setItemDelegate(new ImageEntryItemDelegate());

/*
        self.list.itemSelectionChanged.connect(self.slot_itemSelectionChanged)
        self.list.itemChanged.connect(self.slot_itemChanged)

        self.positionX = self.findChild(QtGui.QLineEdit, "positionX")
        self.positionX.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.positionX.textChanged.connect(self.slot_positionXChanged)
        self.positionY = self.findChild(QtGui.QLineEdit, "positionY")
        self.positionY.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.positionY.textChanged.connect(self.slot_positionYChanged)
        self.width = self.findChild(QtGui.QLineEdit, "width")
        self.width.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.width.textChanged.connect(self.slot_widthChanged)
        self.height = self.findChild(QtGui.QLineEdit, "height")
        self.height.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.height.textChanged.connect(self.slot_heightChanged)
        self.offsetX = self.findChild(QtGui.QLineEdit, "offsetX")
        self.offsetX.setValidator(QtGui.QIntValidator(-9999999, 9999999, self))
        self.offsetX.textChanged.connect(self.slot_offsetXChanged)
        self.offsetY = self.findChild(QtGui.QLineEdit, "offsetY")
        self.offsetY.setValidator(QtGui.QIntValidator(-9999999, 9999999, self))
        self.offsetY.textChanged.connect(self.slot_offsetYChanged)

        self.autoScaledPerImage = self.findChild(QtGui.QComboBox, "autoScaledPerImage")
        self.autoScaledPerImage.currentIndexChanged.connect(self.slot_autoScaledPerImageChanged)
        self.nativeHorzResPerImage = self.findChild(QtGui.QLineEdit, "nativeHorzResPerImage")
        self.nativeHorzResPerImage.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.nativeHorzResPerImage.textEdited.connect(self.slot_nativeResolutionPerImageEdited)
        self.nativeVertResPerImage = self.findChild(QtGui.QLineEdit, "nativeVertResPerImage")
        self.nativeVertResPerImage.setValidator(QtGui.QIntValidator(0, 9999999, self))
        self.nativeVertResPerImage.textEdited.connect(self.slot_nativeResolutionPerImageEdited)

        self.setActiveImageEntry(None)
*/
}

ImagesetEditorDockWidget::~ImagesetEditorDockWidget()
{
    delete ui;
}

// Refreshes the whole list
// Note: User potentially loses selection when this is called!
void ImagesetEditorDockWidget::refresh()
{
    // FIXME: This is really really weird!
    //        If I call list.clear() it crashes when undoing image deletes for some reason
    //        I already spent several hours tracking it down and I couldn't find anything
    //        If I remove items one by one via takeItem, everything works :-/
    ui->list->clear();

    selectionSynchronizationUnderway = true;
    while (ui->list->takeItem(0)) ;
    selectionSynchronizationUnderway = false;
/*

        self.setActiveImageEntry(None)

        self.name.setText(self.imagesetEntry.name)
        self.image.setText(self.imagesetEntry.getAbsoluteImageFile())
        self.autoScaled.setCurrentIndex(self.autoScaled.findText(self.imagesetEntry.autoScaled))
        self.nativeHorzRes.setText(str(self.imagesetEntry.nativeHorzRes))
        self.nativeVertRes.setText(str(self.imagesetEntry.nativeVertRes))

        for imageEntry in self.imagesetEntry.imageEntries:
            item = QtGui.QListWidgetItem()
            item.dockWidget = self
            item.setFlags(QtCore.Qt.ItemIsSelectable |
                          QtCore.Qt.ItemIsEditable |
                          QtCore.Qt.ItemIsEnabled)

            item.imageEntry = imageEntry
            imageEntry.listItem = item
            # nothing is selected (list was cleared) so we don't need to call
            #  the whole updateDockWidget here
            imageEntry.updateListItem()

            self.list.addItem(item)

        # explicitly call the filtering again to make sure it's in sync
        self.filterChanged(self.filterBox.text())
*/
}

void ImagesetEditorDockWidget::onNativeResolutionEdited()
{
/*
        oldHorzRes = self.imagesetEntry.nativeHorzRes
        oldVertRes = self.imagesetEntry.nativeVertRes

        try:
            newHorzRes = int(self.nativeHorzRes.text())
            newVertRes = int(self.nativeVertRes.text())

        except ValueError:
            return

        if oldHorzRes == newHorzRes and oldVertRes == newVertRes:
            return

        cmd = undo.ImagesetChangeNativeResolutionCommand(self.visual, oldHorzRes, oldVertRes, newHorzRes, newVertRes)
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void ImagesetEditorDockWidget::on_name_textEdited(const QString& arg1)
{
/*
        oldName = self.imagesetEntry.name
        newName = self.name.text()

        if oldName == newName:
            return

        cmd = undo.ImagesetRenameCommand(self.visual, oldName, newName)
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void ImagesetEditorDockWidget::on_imageLoad_clicked()
{
/*
        oldImageFile = self.imagesetEntry.imageFile
        newImageFile = self.imagesetEntry.convertToRelativeImageFile(self.image.text())

        if oldImageFile == newImageFile:
            return

        cmd = undo.ImagesetChangeImageCommand(self.visual, oldImageFile, newImageFile)
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void ImagesetEditorDockWidget::on_autoScaled_currentIndexChanged(int index)
{
/*
        oldAutoScaled = self.imagesetEntry.autoScaled
        newAutoScaled = self.autoScaled.currentText()

        if oldAutoScaled == newAutoScaled:
            return

        cmd = undo.ImagesetChangeAutoScaledCommand(self.visual, oldAutoScaled, newAutoScaled)
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void ImagesetEditorDockWidget::on_filterBox_textChanged(const QString &arg1)
{
/*
        # we append star at the beginning and at the end by default (makes property filtering much more practical)
        filter = "*" + filter + "*"
        regex = re.compile(fnmatch.translate(filter), re.IGNORECASE)

        i = 0
        while i < self.list.count():
            listItem = self.list.item(i)
            match = re.match(regex, listItem.text()) is not None
            listItem.setHidden(not match)

            i += 1
*/
}

void ImagesetEditorDockWidget::on_list_itemChanged(QListWidgetItem *item)
{
/*
    def slot_itemChanged(self, item):
        oldName = item.imageEntry.name
        newName = item.text()

        if oldName == newName:
            # most likely caused by RenameCommand doing it's work or is bogus anyways
            return

        cmd = undo.RenameCommand(self.visual, oldName, newName)
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void ImagesetEditorDockWidget::on_list_itemSelectionChanged()
{
/*
    def slot_itemSelectionChanged(self):
        imageEntryNames = self.list.selectedItems()
        if len(imageEntryNames) == 1:
            imageEntry = imageEntryNames[0].imageEntry
            self.setActiveImageEntry(imageEntry)
        else:
            self.setActiveImageEntry(None)

        # we are getting synchronised with the visual editing pane, do not interfere
        if self.selectionSynchronizationUnderway:
            return

        self.selectionUnderway = True
        self.visual.scene().clearSelection()

        imageEntryNames = self.list.selectedItems()
        for imageEntryName in imageEntryNames:
            imageEntry = imageEntryName.imageEntry
            imageEntry.setSelected(True)

        if len(imageEntryNames) == 1:
            imageEntry = imageEntryNames[0].imageEntry
            self.visual.centerOn(imageEntry)

        self.selectionUnderway = False
*/
}

/*

    def setActiveImageEntry(self, imageEntry):
        """Active image entry is the image entry that is selected when there are no
        other image entries selected. It's properties show in the property box.

        Note: Imageset editing doesn't allow multi selection property editing because
              IMO it doesn't make much sense.
        """

        self.activeImageEntry = imageEntry

        self.refreshActiveImageEntry()

    def refreshActiveImageEntry(self):
        """Refreshes the properties of active image entry (from image entry to the property box)
        """

        if not self.activeImageEntry:
            self.positionX.setText("")
            self.positionX.setEnabled(False)
            self.positionY.setText("")
            self.positionY.setEnabled(False)
            self.width.setText("")
            self.width.setEnabled(False)
            self.height.setText("")
            self.height.setEnabled(False)
            self.offsetX.setText("")
            self.offsetX.setEnabled(False)
            self.offsetY.setText("")
            self.offsetY.setEnabled(False)

            self.autoScaledPerImage.setCurrentIndex(0)
            self.autoScaledPerImage.setEnabled(False)
            self.nativeHorzResPerImage.setText("")
            self.nativeHorzResPerImage.setEnabled(False)
            self.nativeVertResPerImage.setText("")
            self.nativeVertResPerImage.setEnabled(False)

        else:
            self.positionX.setText(str(self.activeImageEntry.xpos))
            self.positionX.setEnabled(True)
            self.positionY.setText(str(self.activeImageEntry.ypos))
            self.positionY.setEnabled(True)
            self.width.setText(str(self.activeImageEntry.width))
            self.width.setEnabled(True)
            self.height.setText(str(self.activeImageEntry.height))
            self.height.setEnabled(True)
            self.offsetX.setText(str(self.activeImageEntry.xoffset))
            self.offsetX.setEnabled(True)
            self.offsetY.setText(str(self.activeImageEntry.yoffset))
            self.offsetY.setEnabled(True)

            self.autoScaledPerImage.setCurrentIndex(self.autoScaledPerImage.findText(self.activeImageEntry.autoScaled))
            self.autoScaledPerImage.setEnabled(True)
            self.nativeHorzResPerImage.setText(str(self.activeImageEntry.nativeHorzRes))
            self.nativeHorzResPerImage.setEnabled(True)
            self.nativeVertResPerImage.setText(str(self.activeImageEntry.nativeVertRes))
            self.nativeVertResPerImage.setEnabled(True)

    def keyReleaseEvent(self, event):
        # if we are editing, we should discard key events
        # (delete means delete character, not delete image entry in this context)

        if not self.list.itemDelegate().editing:
            if event.key() == QtCore.Qt.Key_Delete:
                selection = self.visual.scene().selectedItems()

                handled = self.visual.deleteImageEntries(selection)

                if handled:
                    return True

        return super(ImagesetEditorDockWidget, self).keyReleaseEvent(event)

    def metaslot_propertyChangedInt(self, propertyName, newTextValue):
        if not self.activeImageEntry:
            return

        oldValue = getattr(self.activeImageEntry, propertyName)
        newValue = None

        try:
            newValue = int(newTextValue)
        except ValueError:
            # if the string is not a valid integer literal, we allow user to edit some more
            return

        if oldValue == newValue:
            return

        cmd = undo.PropertyEditCommand(self.visual, self.activeImageEntry.name, propertyName, oldValue, newValue)
        self.visual.tabbedEditor.undoStack.push(cmd)

    def metaslot_propertyChangedString(self, propertyName, newValue):
        if not self.activeImageEntry:
            return

        oldValue = getattr(self.activeImageEntry, propertyName)

        if oldValue == newValue:
            return

        cmd = undo.PropertyEditCommand(self.visual, self.activeImageEntry.name, propertyName, oldValue, newValue)
        self.visual.tabbedEditor.undoStack.push(cmd)

    def slot_positionXChanged(self, text):
        self.metaslot_propertyChangedInt("xpos", text)

    def slot_positionYChanged(self, text):
        self.metaslot_propertyChangedInt("ypos", text)

    def slot_widthChanged(self, text):
        self.metaslot_propertyChangedInt("width", text)

    def slot_heightChanged(self, text):
        self.metaslot_propertyChangedInt("height", text)

    def slot_offsetXChanged(self, text):
        self.metaslot_propertyChangedInt("xoffset", text)

    def slot_offsetYChanged(self, text):
        self.metaslot_propertyChangedInt("yoffset", text)

    def slot_autoScaledPerImageChanged(self, index):
        if index == 0:
            # first is the "default" / inheriting state
            text = ""
        else:
            text = self.autoScaledPerImage.currentText()

        self.metaslot_propertyChangedString("autoScaled", text)

    def slot_nativeResolutionPerImageEdited(self, newValue):
        oldHorzRes = self.activeImageEntry.nativeHorzRes
        oldVertRes = self.activeImageEntry.nativeVertRes

        newHorzRes = self.nativeHorzResPerImage.text()
        newVertRes = self.nativeVertResPerImage.text()

        if newHorzRes == "":
            newHorzRes = 0
        if newVertRes == "":
            newVertRes = 0

        try:
            newHorzRes = int(newHorzRes)
            newVertRes = int(newVertRes)

        except ValueError:
            return

        if oldHorzRes == newHorzRes and oldVertRes == newVertRes:
            return

        cmd = undo.PropertyEditCommand(self.visual, self.activeImageEntry.name, "nativeRes", (oldHorzRes, oldVertRes), (newHorzRes, newVertRes))
        self.visual.tabbedEditor.undoStack.push(cmd)
*/
