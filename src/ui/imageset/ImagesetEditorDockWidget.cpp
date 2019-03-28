#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/editors/imageset/ImagesetEditor.h"
#include "ui_ImagesetEditorDockWidget.h"
#include "qitemdelegate.h"
#include "qvalidator.h"
#include "qevent.h"

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

      mutable bool editing = false;
};

ImagesetEditorDockWidget::ImagesetEditorDockWidget(ImagesetVisualMode& visualMode, QWidget* parent)
    : QDockWidget(parent)
    , ui(new Ui::ImagesetEditorDockWidget)
    , _visualMode(visualMode)
{
    ui->setupUi(this);

    ui->image->setInitialDirectoryDelegate([]()
    {
        auto project = CEGUIProjectManager::Instance().getCurrentProject();
        return project ? project->getResourceFilePath("", "imagesets") : "";
    });

    auto uintValidator = new QIntValidator(0, 9999999, this);
    ui->nativeHorzRes->setValidator(uintValidator);
    ui->nativeVertRes->setValidator(uintValidator);
    ui->positionX->setValidator(uintValidator);
    ui->positionY->setValidator(uintValidator);
    ui->width->setValidator(uintValidator);
    ui->height->setValidator(uintValidator);
    ui->nativeVertRes->setValidator(uintValidator);
    ui->nativeVertRes->setValidator(uintValidator);
    ui->nativeHorzResPerImage->setValidator(uintValidator);
    ui->nativeVertResPerImage->setValidator(uintValidator);

    auto intValidator = new QIntValidator(-9999999, 9999999, this);
    ui->offsetX->setValidator(intValidator);
    ui->offsetY->setValidator(intValidator);

    connect(ui->nativeHorzRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionEdited);
    connect(ui->nativeVertRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionEdited);
    connect(ui->nativeHorzResPerImage, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionPerImageEdited);
    connect(ui->nativeVertResPerImage, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::onNativeResolutionPerImageEdited);

    ui->list->setItemDelegate(new ImageEntryItemDelegate());

    setActiveImageEntry(nullptr);
}

ImagesetEditorDockWidget::~ImagesetEditorDockWidget()
{
    delete ui;
}

// Active image entry is the image entry that is selected when there are no
// other image entries selected. It's properties show in the property box.
// NB: Imageset editing doesn't allow multi selection property editing because IMO it doesn't make much sense.
void ImagesetEditorDockWidget::setActiveImageEntry(ImageEntry* entry)
{
    activeImageEntry = entry;
    refreshActiveImageEntry();
}

// Refreshes the properties of active image entry (from image entry to the property box)
void ImagesetEditorDockWidget::refreshActiveImageEntry()
{
    const bool entryIsValid = !!activeImageEntry;

    ui->positionX->setEnabled(entryIsValid);
    ui->positionY->setEnabled(entryIsValid);
    ui->width->setEnabled(entryIsValid);
    ui->height->setEnabled(entryIsValid);
    ui->offsetX->setEnabled(entryIsValid);
    ui->offsetY->setEnabled(entryIsValid);
    ui->autoScaledPerImage->setEnabled(entryIsValid);
    ui->nativeHorzResPerImage->setEnabled(entryIsValid);
    ui->nativeVertResPerImage->setEnabled(entryIsValid);

    ui->positionX->setText(activeImageEntry ? QString::number(activeImageEntry->pos().x()) : "");
    ui->positionY->setText(activeImageEntry ? QString::number(activeImageEntry->pos().y()) : "");
    ui->width->setText(activeImageEntry ? QString::number(activeImageEntry->rect().width()) : "");
    ui->height->setText(activeImageEntry ? QString::number(activeImageEntry->rect().height()) : "");
    ui->offsetX->setText(activeImageEntry ? QString::number(activeImageEntry->offsetX()) : "");
    ui->offsetY->setText(activeImageEntry ? QString::number(activeImageEntry->offsetY()) : "");
    ui->autoScaledPerImage->setCurrentIndex(activeImageEntry ? ui->autoScaledPerImage->findText(activeImageEntry->getAutoScaled()) : 0);
    ui->nativeHorzResPerImage->setText(activeImageEntry ? QString::number(activeImageEntry->getNativeHorzRes()) : "");
    ui->nativeVertResPerImage->setText(activeImageEntry ? QString::number(activeImageEntry->getNativeVertRes()) : "");
}

void ImagesetEditorDockWidget::refreshImagesetInfo()
{
    if (!imagesetEntry) return;

    ui->name->setText(imagesetEntry->name());
    ui->image->setText(imagesetEntry->getAbsoluteImageFile());
    ui->autoScaled->setCurrentIndex(ui->autoScaled->findText(imagesetEntry->getAutoScaled()));
    ui->nativeHorzRes->setText(QString::number(imagesetEntry->getNativeHorzRes()));
    ui->nativeVertRes->setText(QString::number(imagesetEntry->getNativeVertRes()));
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

    setActiveImageEntry(nullptr);

    assert(imagesetEntry);

    refreshImagesetInfo();

    for (ImageEntry* imageEntry : imagesetEntry->getImageEntries())
    {
        auto item = new QListWidgetItem();
        item->setData(Qt::UserRole + 1, QVariant::fromValue(this));
        item->setData(Qt::UserRole + 2, QVariant::fromValue(imageEntry));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        imageEntry->setListItem(item);
        imageEntry->updateListItem(); // Nothing is selected (list was cleared) so we don't need to call the whole updateDockWidget here
        ui->list->addItem(item);
    }

    // Explicitly call the filtering again to make sure it's in sync
    on_filterBox_textChanged(ui->filterBox->text());
}

void ImagesetEditorDockWidget::scrollToEntry(ImageEntry* entry)
{
    if (entry) ui->list->scrollToItem(entry->getListItem());
}

// Focuses into image list filter. This potentially allows the user to just press a shortcut to find images,
// instead of having to reach for a mouse.
void ImagesetEditorDockWidget::focusImageListFilterBox()
{
    // Selects all contents of the filter so that user can replace that with their search phrase
    ui->filterBox->selectAll();
    // Sets focus so that typing puts text into the filter box without clicking
    ui->filterBox->setFocus();
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

void ImagesetEditorDockWidget::onNativeResolutionPerImageEdited()
{
    auto oldHorzRes = activeImageEntry->getNativeHorzRes();
    auto oldVertRes = activeImageEntry->getNativeVertRes();

    auto newHorzResStr = ui->nativeHorzResPerImage->text();
    auto newVertResStr = ui->nativeVertResPerImage->text();
    if (newHorzResStr.isEmpty()) newHorzResStr = "0";
    if (newVertResStr.isEmpty()) newVertResStr = "0";

    bool ok = false;
    auto newHorzRes = newHorzResStr.toInt(&ok);
    if (!ok) return;
    auto newVertRes = newVertResStr.toInt(&ok);
    if (!ok) return;

    if (oldHorzRes == newHorzRes && oldVertRes == newVertRes) return;

    _visualMode.getEditor().getUndoStack()->push(new ImagePropertyEditCommand(_visualMode, activeImageEntry->name(),
                                                                              "nativeRes",
                                                                              QPoint(oldHorzRes, oldVertRes),
                                                                              QPoint(newHorzRes, newVertRes)));
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

void ImagesetEditorDockWidget::on_autoScaledPerImage_currentIndexChanged(int index)
{
    // First is the "default" / inheriting state
    QString text = (index == 0) ? "" : ui->autoScaledPerImage->currentText();
    onStringPropertyChanged("autoScaled", text);
}

void ImagesetEditorDockWidget::on_filterBox_textChanged(const QString& arg1)
{
    // We append star at the beginning and at the end by default (makes property filtering much more practical)
    QString filter = "*" + arg1 + "*";
/*
        regex = re.compile(fnmatch.translate(filter), re.IGNORECASE)

        i = 0
        while i < self.list.count():
            listItem = self.list.item(i)
            match = re.match(regex, listItem.text()) is not None
            listItem.setHidden(not match)

            i += 1
*/
}

void ImagesetEditorDockWidget::on_list_itemChanged(QListWidgetItem* item)
{
    auto oldName = item->data(Qt::UserRole + 2).value<ImageEntry*>()->name();
    auto newName = item->text();

    // Most likely caused by RenameCommand doing it's work or is bogus anyways
    if (oldName == newName) return;

    _visualMode.getEditor().getUndoStack()->push(new ImageRenameCommand(_visualMode, oldName, newName));
}

void ImagesetEditorDockWidget::on_list_itemSelectionChanged()
{
    auto imageEntryNames = ui->list->selectedItems();
    setActiveImageEntry(imageEntryNames.empty() ? nullptr : imageEntryNames[0]->data(Qt::UserRole + 2).value<ImageEntry*>());

    // We are getting synchronised with the visual editing pane, do not interfere
    if (selectionSynchronizationUnderway) return;

    selectionUnderway = true;

    _visualMode.scene()->clearSelection();

    auto imageEntryItems = ui->list->selectedItems();
    for (auto&& imageEntryItem : imageEntryItems)
    {
        auto imageEntry = imageEntryItem->data(Qt::UserRole + 2).value<ImageEntry*>();
        imageEntry->setSelected(true);
    }

    if (imageEntryItems.size() == 1)
    {
        auto imageEntry = imageEntryItems[0]->data(Qt::UserRole + 2).value<ImageEntry*>();
        _visualMode.centerOn(imageEntry);
    }

    selectionUnderway = false;
}

void ImagesetEditorDockWidget::on_positionX_textChanged(const QString& arg1)
{
    onIntPropertyChanged("xpos", arg1);
}

void ImagesetEditorDockWidget::on_positionY_textChanged(const QString& arg1)
{
    onIntPropertyChanged("ypos", arg1);
}

void ImagesetEditorDockWidget::on_width_textChanged(const QString& arg1)
{
    onIntPropertyChanged("width", arg1);
}

void ImagesetEditorDockWidget::on_height_textChanged(const QString& arg1)
{
    onIntPropertyChanged("height", arg1);
}

void ImagesetEditorDockWidget::on_offsetX_textChanged(const QString& arg1)
{
    onIntPropertyChanged("xoffset", arg1);
}

void ImagesetEditorDockWidget::on_offsetY_textChanged(const QString& arg1)
{
    onIntPropertyChanged("yoffset", arg1);
}

void ImagesetEditorDockWidget::onIntPropertyChanged(const QString& name, const QString& valueString)
{
    if (!activeImageEntry) return;

    bool ok = false;
    int newValue = valueString.toInt(&ok);
    if (!ok) return; // If the string is not a valid integer literal, we allow user to edit some more

    QVariant oldValue = activeImageEntry->getProperty(name);
    if (oldValue.toInt() == newValue) return;

    _visualMode.getEditor().getUndoStack()->push(new ImagePropertyEditCommand(_visualMode, activeImageEntry->name(),
                                                                              name, oldValue, newValue));
}

void ImagesetEditorDockWidget::onStringPropertyChanged(const QString& name, const QString& newValue)
{
    if (!activeImageEntry) return;

    QVariant oldValue = activeImageEntry->getProperty(name);
    if (oldValue.toString() == newValue) return;

    _visualMode.getEditor().getUndoStack()->push(new ImagePropertyEditCommand(_visualMode, activeImageEntry->name(),
                                                                              name, oldValue, newValue));
}

void ImagesetEditorDockWidget::keyReleaseEvent(QKeyEvent* event)
{
    // If we are editing, we should discard key events
    // (delete means delete character, not delete image entry in this context)
    if (!static_cast<ImageEntryItemDelegate*>(ui->list->itemDelegate())->editing)
    {
        if (event->key() == Qt::Key_Delete)
        {
            if (_visualMode.deleteSelectedImageEntries()) return;
        }
    }

    QDockWidget::keyReleaseEvent(event);
}
