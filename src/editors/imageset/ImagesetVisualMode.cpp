#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/util/Settings.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/Application.h"
#include "qopenglwidget.h"
#include "qclipboard.h"
#include "qmimedata.h"
#include "qevent.h"
#include "qdom.h"

constexpr qreal newImageHalfSize = 25.0;

ImagesetVisualMode::ImagesetVisualMode(MultiModeEditor& editor)
    : IEditMode(editor)
    , lastCursorPosition(newImageHalfSize, newImageHalfSize)
{
    wheelZoomEnabled = true;
    middleButtonDragScrollEnabled = true;

    setScene(new QGraphicsScene());

    setFocusPolicy(Qt::ClickFocus);
    setFrameStyle(QFrame::NoFrame);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    if (settings->getEntryValue("imageset/visual/partial_updates").toBool())
    {
        // the commented lines are possible optimisation, I found out that they don't really
        // speed it up in a noticeable way so I commented them out

        //setOptimizationFlag(DontSavePainterState, true);
        //setOptimizationFlag(DontAdjustForAntialiasing, true);
        //setCacheMode(CacheBackground);
        setViewportUpdateMode(MinimalViewportUpdate);
        //setRenderHint(QPainter::Antialiasing, false);
        //setRenderHint(QPainter::TextAntialiasing, false);
        //setRenderHint(QPainter::SmoothPixmapTransform, false);
    }
    else
    {
        // use OpenGL for view redrawing
        // depending on the platform and hardware this may be faster or slower
        setViewport(new QOpenGLWidget());
        setViewportUpdateMode(FullViewportUpdate);
    }

    setDragMode(RubberBandDrag);
    setBackgroundBrush(QBrush(Qt::lightGray));

    connect(scene(), &QGraphicsScene::selectionChanged, this, &ImagesetVisualMode::slot_selectionChanged);

    dockWidget = new ImagesetEditorDockWidget(*this);

    setupActions();
}

void ImagesetVisualMode::setupActions()
{
/*
        self.connectionGroup = action.ConnectionGroup(action.ActionManager.instance)

        self.editOffsetsAction = action.getAction("imageset/edit_offsets")
        self.connectionGroup.add(self.editOffsetsAction, receiver = self.slot_toggleEditOffsets, signalName = "toggled")

        self.cycleOverlappingAction = action.getAction("imageset/cycle_overlapping")
        self.connectionGroup.add(self.cycleOverlappingAction, receiver = self.cycleOverlappingImages)

        self.createImageAction = action.getAction("imageset/create_image")
        self.connectionGroup.add(self.createImageAction, receiver = self.createImageAtCursor)

        self.duplicateSelectedImagesAction = action.getAction("imageset/duplicate_image")
        self.connectionGroup.add(self.duplicateSelectedImagesAction, receiver = self.duplicateSelectedImageEntries)

        self.focusImageListFilterBoxAction = action.getAction("imageset/focus_image_list_filter_box")
        self.connectionGroup.add(self.focusImageListFilterBoxAction, receiver = lambda: self.dockWidget.focusImageListFilterBox())

        self.toolBar = QtGui.QToolBar("Imageset")
        self.toolBar.setObjectName("ImagesetToolbar")
        self.toolBar.setIconSize(QtCore.QSize(32, 32))

        self.toolBar.addAction(self.createImageAction)
        self.toolBar.addAction(self.duplicateSelectedImagesAction)
        self.toolBar.addSeparator() // ---------------------------
        self.toolBar.addAction(self.editOffsetsAction)
        self.toolBar.addAction(self.cycleOverlappingAction)

        self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)

        self.contextMenu = QtGui.QMenu(self)
        self.customContextMenuRequested.connect(self.slot_customContextMenu)

        self.contextMenu.addAction(self.createImageAction)
        self.contextMenu.addAction(self.duplicateSelectedImagesAction)
        self.contextMenu.addAction(action.getAction("all_editors/delete"))
        self.contextMenu.addSeparator() // ----------------------
        self.contextMenu.addAction(self.cycleOverlappingAction)
        self.contextMenu.addSeparator() // ----------------------
        self.contextMenu.addAction(action.getAction("all_editors/zoom_in"))
        self.contextMenu.addAction(action.getAction("all_editors/zoom_out"))
        self.contextMenu.addAction(action.getAction("all_editors/zoom_reset"))
        self.contextMenu.addSeparator() // ----------------------
        self.contextMenu.addAction(self.editOffsetsAction)
*/
}

void ImagesetVisualMode::loadImagesetEntryFromElement(const QDomElement& xmlRoot)
{
    scene()->clear();

    imagesetEntry = new ImagesetEntry(*this);
    imagesetEntry->loadFromElement(xmlRoot);
    scene()->addItem(imagesetEntry);

    refreshSceneRect();

    dockWidget->setImagesetEntry(imagesetEntry);
    dockWidget->refresh();
}

void ImagesetVisualMode::refreshSceneRect()
{
    // The reason to make the bounding rect 100px bigger on all the sides is to make
    // middle button drag scrolling easier (you can put the image where you want without
    // running out of scene
    auto boundingRect = imagesetEntry->boundingRect();
    boundingRect.adjust(-100, -100, 100, 100);
    scene()->setSceneRect(boundingRect);
}

// Centre position is the position of the centre of the newly created image,
// the newly created image will then 'encapsulate' the centrepoint
void ImagesetVisualMode::createImageEntry(QPointF pos)
{
    // Find an unique image name
    QString name = "NewImage";
    int index = 1;

    while (true)
    {
        ImageEntry* imageEntry = imagesetEntry->getImageEntry(name);
        if (!imageEntry) break;
        name = QString("NewImage_%i").arg(index++);
    }

    QPointF imgPos = pos - QPointF(newImageHalfSize, newImageHalfSize);
    QSizeF imgSize(2.0 * newImageHalfSize, 2.0 * newImageHalfSize);
    QPoint imgOffset(0, 0);
    _editor.getUndoStack()->push(new ImagesetCreateCommand(*this, name, imgPos, imgSize, imgOffset));
}

bool ImagesetVisualMode::moveImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF delta)
{
    if (imageEntries.empty() || delta.manhattanLength() <= 0.0) return false;

    std::vector<ImagesetMoveCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetMoveCommand::Record rec;
        rec.name = imageEntry->name();
        rec.oldPos = imageEntry->pos();
        rec.newPos = imageEntry->pos() + delta;
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetMoveCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::resizeImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF topLeftDelta, QPointF bottomRightDelta)
{
    if (imageEntries.empty() || (topLeftDelta.manhattanLength() <= 0.0 && bottomRightDelta.manhattanLength() <= 0.0)) return false;

    std::vector<ImagesetGeometryChangeCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetGeometryChangeCommand::Record rec;
        rec.name = imageEntry->name();
        rec.oldPos = imageEntry->pos();
        rec.newPos = imageEntry->pos() - topLeftDelta;
        rec.oldRect = imageEntry->rect();
        rec.newRect = imageEntry->rect();
        rec.newRect.setBottomRight(rec.newRect.bottomRight() - topLeftDelta + bottomRightDelta);
        if (rec.newRect.width() < 1.0) rec.newRect.setWidth(1.0);
        if (rec.newRect.height() < 1.0) rec.newRect.setHeight(1.0);
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetGeometryChangeCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::deleteImageEntries(const std::vector<ImageEntry*>& imageEntries)
{
    if (imageEntries.empty()) return false;

    std::vector<ImagesetDeleteCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetDeleteCommand::Record rec;
        rec.name = imageEntry->name();
        rec.pos = imageEntry->pos();
        rec.size = imageEntry->rect().size();
        rec.offset = QPoint(imageEntry->offsetX(), imageEntry->offsetY());
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetDeleteCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::deleteSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    std::vector<ImageEntry*> imageEntries;
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (entry) imageEntries.push_back(entry);
    }

    return deleteImageEntries(imageEntries);
}

bool ImagesetVisualMode::duplicateImageEntries(const std::vector<ImageEntry*>& imageEntries)
{
    if (imageEntries.empty()) return false;

    std::vector<ImagesetDuplicateCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetDuplicateCommand::Record rec;
        rec.name = getNewImageName(imageEntry->name());
        rec.pos = imageEntry->pos();
        rec.size = imageEntry->rect().size();
        rec.offset = QPoint(imageEntry->offsetX(), imageEntry->offsetY());
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetDuplicateCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::duplicateSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    std::vector<ImageEntry*> imageEntries;
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (entry) imageEntries.push_back(entry);
    }

    return duplicateImageEntries(imageEntries);
}

bool ImagesetVisualMode::cut()
{
    if (!copy()) return false;
    deleteSelectedImageEntries();
    return true;
}

bool ImagesetVisualMode::copy()
{
    auto selection = scene()->selectedItems();

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (!entry) continue;

        stream << entry->name();
        stream << entry->pos();
        stream << entry->rect();
        stream << QPoint(entry->offsetX(), entry->offsetY());
    }

    if (!bytes.size()) return false;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-ceed-imageset-image-list", bytes);
    QApplication::clipboard()->setMimeData(mimeData);
    return true;
}

bool ImagesetVisualMode::paste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData->hasFormat("application/x-ceed-imageset-image-list")) return false;

    std::vector<ImagesetPasteCommand::Record> undo;
    std::vector<QString> newNames;

    QByteArray bytes = mimeData->data("application/x-ceed-imageset-image-list");
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        QString name;
        stream >> name;

        ImagesetPasteCommand::Record rec;
        rec.name = getNewImageName(name);

        newNames.push_back(rec.name);

        stream >> rec.pos;
        stream >> rec.size;
        stream >> rec.offset;
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetPasteCommand(*this, std::move(undo)));

    // Select just the pasted image definitions for convenience
    scene()->clearSelection();
    for (const auto& name : newNames)
        imagesetEntry->getImageEntry(name)->setSelected(true);

    return true;
}

// Returns an image name that is not used in this imageset
QString ImagesetVisualMode::getNewImageName(const QString& desiredName, QString copyPrefix, QString copySuffix)
{
    // Try the desired name exactly
    if (!imagesetEntry->getImageEntry(desiredName)) return desiredName;

    // Try with prefix and suffix
    QString name = copyPrefix + desiredName + copySuffix;
    if (!imagesetEntry->getImageEntry(name)) return name;

    // We're forced to append a counter, start with number 2 (_copy2, copy3, etc.)
    int counter = 2;
    while (true)
    {
        QString indexedName = name + QString::number(counter);
        if (!imagesetEntry->getImageEntry(indexedName)) return indexedName;
        ++counter;
    }
}

bool ImagesetVisualMode::cycleOverlappingImages()
{
    auto selection = scene()->selectedItems();
    if (selection.size() != 1) return false;

    auto rect = selection[0]->boundingRect();
    rect.translate(selection[0]->pos());

    auto overlappingItems = scene()->items(rect);

    // First we stack everything before our current selection
    ImageEntry* successor = nullptr;
    for (QGraphicsItem* item : overlappingItems)
    {
        if (item == selection[0] || item->parentItem() != selection[0]->parentItem())
            continue;

        if (!successor) successor = dynamic_cast<ImageEntry*>(item);
    }

    if (successor)
    {
        for (QGraphicsItem* item : overlappingItems)
        {
            if (item == successor || item->parentItem() != successor->parentItem())
                continue;

            successor->stackBefore(item);
        }

        // We deselect current
        selection[0]->setSelected(false);
        auto selectedImage = dynamic_cast<ImageEntry*>(selection[0]);
        if (selectedImage) selectedImage->hoverLeaveEvent(nullptr);

        // And select what was at the bottom (thus getting this to the top)
        successor->setSelected(true);
        successor->hoverEnterEvent(nullptr);
    }

    return true;
}

void ImagesetVisualMode::slot_selectionChanged()
{
    // If dockWidget is changing the selection, back off
    if (dockWidget->isSelectionUnderway()) return;

    auto selectedItems = scene()->selectedItems();
    if (selectedItems.size() == 1)
    {
        ImageEntry* entry = dynamic_cast<ImageEntry*>(selectedItems[0]);
        dockWidget->scrollToEntry(entry);
    }
}

void ImagesetVisualMode::mouseMoveEvent(QMouseEvent* event)
{
    lastCursorPosition = mapToScene(event->pos());
    ResizableGraphicsView::mouseMoveEvent(event);
}

/*
    def rebuildEditorMenu(self, editorMenu):
        """Adds actions to the editor menu"""
        // similar to the toolbar, includes the focus filter box action
        editorMenu.addAction(self.createImageAction)
        editorMenu.addAction(self.duplicateSelectedImagesAction)
        editorMenu.addSeparator() // ---------------------------
        editorMenu.addAction(self.cycleOverlappingAction)
        editorMenu.addSeparator() // ---------------------------
        editorMenu.addAction(self.editOffsetsAction)
        editorMenu.addSeparator() // ---------------------------
        editorMenu.addAction(self.focusImageListFilterBoxAction)

    def showEvent(self, event):
        self.dockWidget.setEnabled(True)
        self.toolBar.setEnabled(True)
        if self.tabbedEditor.editorMenu() is not None:
            self.tabbedEditor.editorMenu().menuAction().setEnabled(True)

        // connect all our actions
        self.connectionGroup.connectAll()
        // call this every time the visual editing is shown to sync all entries up
        self.slot_toggleEditOffsets(self.editOffsetsAction.isChecked())

        super(VisualEditing, self).showEvent(event)

    def hideEvent(self, event):
        // disconnected all our actions
        self.connectionGroup.disconnectAll()

        self.dockWidget.setEnabled(False)
        self.toolBar.setEnabled(False)
        if self.tabbedEditor.editorMenu() is not None:
            self.tabbedEditor.editorMenu().menuAction().setEnabled(False)

        super(VisualEditing, self).hideEvent(event)

    def mousePressEvent(self, event):
        super(VisualEditing, self).mousePressEvent(event)

        if event.buttons() & QtCore.Qt.LeftButton:
            for selectedItem in self.scene().selectedItems():
                // selectedItem could be ImageEntry or ImageOffset!
                selectedItem.potentialMove = True
                selectedItem.oldPosition = None

    def mouseReleaseEvent(self, event):
        """When mouse is released, we have to check what items were moved and resized.

        AFAIK Qt doesn't give us any move finished notification so I do this manually
        """

        super(VisualEditing, self).mouseReleaseEvent(event)

        // moving
        moveImageNames = []
        moveImageOldPositions = {}
        moveImageNewPositions = {}

        moveOffsetNames = []
        moveOffsetOldPositions = {}
        moveOffsetNewPositions = {}

        // resizing
        resizeImageNames = []
        resizeImageOldPositions = {}
        resizeImageOldRects = {}
        resizeImageNewPositions = {}
        resizeImageNewRects = {}

        // we have to "expand" the items, adding parents of resizing handles
        // instead of the handles themselves
        expandedSelectedItems = []
        for selectedItem in self.scene().selectedItems():
            if isinstance(selectedItem, elements.ImageEntry):
                expandedSelectedItems.append(selectedItem)
            elif isinstance(selectedItem, elements.ImageOffset):
                expandedSelectedItems.append(selectedItem)
            elif isinstance(selectedItem, resizable.ResizingHandle):
                expandedSelectedItems.append(selectedItem.parentItem())

        for selectedItem in expandedSelectedItems:
            if isinstance(selectedItem, elements.ImageEntry):
                if selectedItem.oldPosition:
                    if selectedItem.mouseOver:
                        // show the label again if mouse is over because moving finished
                        selectedItem.label.setVisible(True)

                    // only include that if the position really changed
                    if selectedItem.oldPosition != selectedItem.pos():
                        moveImageNames.append(selectedItem.name)
                        moveImageOldPositions[selectedItem.name] = selectedItem.oldPosition
                        moveImageNewPositions[selectedItem.name] = selectedItem.pos()

                if selectedItem.resized:
                    // only include that if the position or rect really changed
                    if selectedItem.resizeOldPos != selectedItem.pos() or selectedItem.resizeOldRect != selectedItem.rect():
                        resizeImageNames.append(selectedItem.name)
                        resizeImageOldPositions[selectedItem.name] = selectedItem.resizeOldPos
                        resizeImageOldRects[selectedItem.name] = selectedItem.resizeOldRect
                        resizeImageNewPositions[selectedItem.name] = selectedItem.pos()
                        resizeImageNewRects[selectedItem.name] = selectedItem.rect()

                selectedItem.potentialMove = False
                selectedItem.oldPosition = None
                selectedItem.resized = False

            elif isinstance(selectedItem, elements.ImageOffset):
                if selectedItem.oldPosition:
                    // only include that if the position really changed
                    if selectedItem.oldPosition != selectedItem.pos():
                        moveOffsetNames.append(selectedItem.imageEntry.name)
                        moveOffsetOldPositions[selectedItem.imageEntry.name] = selectedItem.oldPosition
                        moveOffsetNewPositions[selectedItem.imageEntry.name] = selectedItem.pos()

                selectedItem.potentialMove = False
                selectedItem.oldPosition = None

        // NOTE: It should never happen that more than one of these sets is populated
        //       User moves images XOR moves offsets XOR resizes images
        //
        //       I don't do elif for robustness though, who knows what can happen ;-)

        if len(moveImageNames) > 0:
            cmd = undo.MoveCommand(self, moveImageNames, moveImageOldPositions, moveImageNewPositions)
            self.tabbedEditor.undoStack.push(cmd)

        if len(moveOffsetNames) > 0:
            cmd = undo.OffsetMoveCommand(self, moveOffsetNames, moveOffsetOldPositions, moveOffsetNewPositions)
            self.tabbedEditor.undoStack.push(cmd)

        if len(resizeImageNames) > 0:
            cmd = undo.GeometryChangeCommand(self, resizeImageNames, resizeImageOldPositions, resizeImageOldRects, resizeImageNewPositions, resizeImageNewRects)
            self.tabbedEditor.undoStack.push(cmd)

    def keyReleaseEvent(self, event):
        // TODO: offset keyboard handling

        handled = False

        if event.key() in [QtCore.Qt.Key_A, QtCore.Qt.Key_D, QtCore.Qt.Key_W, QtCore.Qt.Key_S]:
            selection = []

            for item in self.scene().selectedItems():
                if item in selection:
                    continue

                if isinstance(item, elements.ImageEntry):
                    selection.append(item)

                elif isinstance(item, resizable.ResizingHandle):
                    parent = item.parentItem()
                    if not parent in selection:
                        selection.append(parent)

            if len(selection) > 0:
                delta = QtCore.QPointF()

                if event.key() == QtCore.Qt.Key_A:
                    delta += QtCore.QPointF(-1, 0)
                elif event.key() == QtCore.Qt.Key_D:
                    delta += QtCore.QPointF(1, 0)
                elif event.key() == QtCore.Qt.Key_W:
                    delta += QtCore.QPointF(0, -1)
                elif event.key() == QtCore.Qt.Key_S:
                    delta += QtCore.QPointF(0, 1)

                if event.modifiers() & QtCore.Qt.ControlModifier:
                    delta *= 10

                if event.modifiers() & QtCore.Qt.ShiftModifier:
                    handled = self.resizeImageEntries(selection, QtCore.QPointF(0, 0), delta)
                else:
                    handled = self.moveImageEntries(selection, delta)

        elif event.key() == QtCore.Qt.Key_Q:
            handled = self.cycleOverlappingImages()

        elif event.key() == QtCore.Qt.Key_Delete:
            handled = self.deleteSelectedImageEntries()

        if not handled:
            super(VisualEditing, self).keyReleaseEvent(event)

        else:
            event.accept()


    def slot_toggleEditOffsets(self, enabled):
        self.scene().clearSelection()

        if self.imagesetEntry is not None:
            self.imagesetEntry.showOffsets = enabled

    def slot_customContextMenu(self, point):
        self.contextMenu.exec_(self.mapToGlobal(point))
*/
