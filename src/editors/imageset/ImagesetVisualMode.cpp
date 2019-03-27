#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/util/Settings.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/Application.h"
#include "qopenglwidget.h"
#include "qdom.h"

ImagesetVisualMode::ImagesetVisualMode(MultiModeEditor& editor)
    : IEditMode(editor)
{
    wheelZoomEnabled = true;
    middleButtonDragScrollEnabled = true;

    // Reset to unreachable value
    lastMousePosition.setX(-10000);
    lastMousePosition.setY(-10000);

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
        self.connectionGroup.add(self.focusImageListFilterBoxAction, receiver = lambda: self.focusImageListFilterBox())

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

    def resizeImageEntries(self, imageEntries, topLeftDelta, bottomRightDelta):
        if (topLeftDelta.manhattanLength() > 0 or bottomRightDelta.manhattanLength() > 0) and len(imageEntries) > 0:
            imageNames = []
            oldPositions = {}
            oldRects = {}
            newPositions = {}
            newRects = {}

            for imageEntry in imageEntries:

                imageNames.append(imageEntry.name)
                oldPositions[imageEntry.name] = imageEntry.pos()
                newPositions[imageEntry.name] = imageEntry.pos() - topLeftDelta
                oldRects[imageEntry.name] = imageEntry.rect()

                newRect = imageEntry.rect()
                newRect.setBottomRight(newRect.bottomRight() - topLeftDelta + bottomRightDelta)

                if newRect.width() < 1:
                    newRect.setWidth(1)
                if newRect.height() < 1:
                    newRect.setHeight(1)

                newRects[imageEntry.name] = newRect

            cmd = undo.GeometryChangeCommand(self, imageNames, oldPositions, oldRects, newPositions, newRects)
            self.tabbedEditor.undoStack.push(cmd)

            // we handled this
            return True

        // we didn't handle this
        return False

    def cycleOverlappingImages(self):
        selection = self.scene().selectedItems()

        if len(selection) == 1:
            rect = selection[0].boundingRect()
            rect.translate(selection[0].pos())

            overlappingItems = self.scene().items(rect)

            // first we stack everything before our current selection
            successor = None
            for item in overlappingItems:
                if item == selection[0] or item.parentItem() != selection[0].parentItem():
                    continue

                if not successor and isinstance(item, elements.ImageEntry):
                    successor = item

            if successor:
                for item in overlappingItems:
                    if item == successor or item.parentItem() != successor.parentItem():
                        continue

                    successor.stackBefore(item)

                // we deselect current
                selection[0].setSelected(False)
                selection[0].hoverLeaveEvent(None)
                // and select what was at the bottom (thus getting this to the top)
                successor.setSelected(True)
                successor.hoverEnterEvent(None)

            // we handled this
            return True

        // we didn't handle this
        return False

    def createImage(self, centrePositionX, centrePositionY):
        """Centre position is the position of the centre of the newly created image,
        the newly created image will then 'encapsulate' the centrepoint
        """

        // find a unique image name
        name = "NewImage"
        index = 1

        while True:
            found = False
            for imageEntry in self.imagesetEntry.imageEntries:
                if imageEntry.name == name:
                    found = True
                    break

            if found:
                name = "NewImage_%i" % (index)
                index += 1
            else:
                break

        halfSize = 25

        xpos = centrePositionX - halfSize
        ypos = centrePositionY - halfSize
        width = 2 * halfSize
        height = 2 * halfSize
        xoffset = 0
        yoffset = 0

        cmd = undo.CreateCommand(self, name, xpos, ypos, width, height, xoffset, yoffset)
        self.tabbedEditor.undoStack.push(cmd)

    def createImageAtCursor(self):
        assert(self.lastMousePosition is not None)
        sceneCoordinates = self.mapToScene(self.lastMousePosition)

        self.createImage(int(sceneCoordinates.x()), int(sceneCoordinates.y()))

    def getImageByName(self, name):
        for imageEntry in self.imagesetEntry.imageEntries:
            if imageEntry.name == name:
                return imageEntry
        return None

    def getNewImageName(self, desiredName, copyPrefix = "", copySuffix = "_copy"):
        """Returns an image name that is not used in this imageset

        TODO: Can be used for the copy-paste functionality too
        """

        // Try the desired name exactly
        if self.getImageByName(desiredName) is None:
            return desiredName

        // Try with prefix and suffix
        desiredName = copyPrefix + desiredName + copySuffix
        if self.getImageByName(desiredName) is None:
            return desiredName

        // We're forced to append a counter, start with number 2 (_copy2, copy3, etc.)
        counter = 2
        while True:
            tmpName = desiredName + str(counter)
            if self.getImageByName(tmpName) is None:
                return tmpName
            counter += 1

    def duplicateImageEntries(self, imageEntries):
        if len(imageEntries) > 0:
            newNames = []

            newPositions = {}
            newRects = {}
            newOffsets = {}

            for imageEntry in imageEntries:
                newName = self.getNewImageName(imageEntry.name)
                newNames.append(newName)

                newPositions[newName] = imageEntry.pos()
                newRects[newName] = imageEntry.rect()
                newOffsets[newName] = imageEntry.offset.pos()

            cmd = undo.DuplicateCommand(self, newNames, newPositions, newRects, newOffsets)
            self.tabbedEditor.undoStack.push(cmd)

            return True

        else:
            // we didn't handle this
            return False

    def duplicateSelectedImageEntries(self):
        selection = self.scene().selectedItems()

        imageEntries = []
        for item in selection:
            if isinstance(item, elements.ImageEntry):
                imageEntries.append(item)

        return self.duplicateImageEntries(imageEntries)

    def deleteImageEntries(self, imageEntries):
        if len(imageEntries) > 0:
            oldNames = []

            oldPositions = {}
            oldRects = {}
            oldOffsets = {}

            for imageEntry in imageEntries:
                oldNames.append(imageEntry.name)

                oldPositions[imageEntry.name] = imageEntry.pos()
                oldRects[imageEntry.name] = imageEntry.rect()
                oldOffsets[imageEntry.name] = imageEntry.offset.pos()

            cmd = undo.DeleteCommand(self, oldNames, oldPositions, oldRects, oldOffsets)
            self.tabbedEditor.undoStack.push(cmd)

            return True

        else:
            // we didn't handle this
            return False

    def deleteSelectedImageEntries(self):
        selection = self.scene().selectedItems()

        imageEntries = []
        for item in selection:
            if isinstance(item, elements.ImageEntry):
                imageEntries.append(item)

        return self.deleteImageEntries(imageEntries)

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

    def mouseMoveEvent(self, event):
        self.lastMousePosition = event.pos()

        super(VisualEditing, self).mouseMoveEvent(event)

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

    def focusImageListFilterBox(self):
        """Focuses into image list filter

        This potentially allows the user to just press a shortcut to find images,
        instead of having to reach for a mouse.
        """

        filterBox = self.dockWidget.filterBox
        // selects all contents of the filter so that user can replace that with their search phrase
        filterBox.selectAll()
        // sets focus so that typing puts text into the filter box without clicking
        filterBox.setFocus()

    def performCut(self):
        if self.performCopy():
            self.deleteSelectedImageEntries()
            return True

        return False

    def performCopy(self):
        selection = self.scene().selectedItems()
        if len(selection) == 0:
            return False

        copyNames = []
        copyPositions = {}
        copyRects = {}
        copyOffsets = {}
        for item in selection:
            if isinstance(item, elements.ImageEntry):
                copyNames.append(item.name)
                copyPositions[item.name] = item.pos()
                copyRects    [item.name] = item.rect()
                copyOffsets  [item.name] = item.offset.pos()
        if len(copyNames) == 0:
            return False

        imageData = []
        imageData.append(copyNames)
        imageData.append(copyPositions)
        imageData.append(copyRects)
        imageData.append(copyOffsets)

        data = QtCore.QMimeData()
        data.setData("application/x-ceed-imageset-image-list", QtCore.QByteArray(cPickle.dumps(imageData)))
        QtGui.QApplication.clipboard().setMimeData(data)

        return True

    def performPaste(self):
        data = QtGui.QApplication.clipboard().mimeData()
        if not data.hasFormat("application/x-ceed-imageset-image-list"):
            return False

        imageData = cPickle.loads(data.data("application/x-ceed-imageset-image-list").data())
        if len(imageData) != 4:
            return False

        newNames = []
        newPositions = {}
        newRects = {}
        newOffsets = {}
        for copyName in imageData[0]:
            newName = self.getNewImageName(copyName)
            newNames.append(newName)
            newPositions[newName] = imageData[1][copyName]
            newRects    [newName] = imageData[2][copyName]
            newOffsets  [newName] = imageData[3][copyName]
        if len(newNames) == 0:
            return False

        cmd = undo.PasteCommand(self, newNames, newPositions, newRects, newOffsets)
        self.tabbedEditor.undoStack.push(cmd)

        // select just the pasted image definitions for convenience
        self.scene().clearSelection()
        for name in newNames:
            self.imagesetEntry.getImageEntry(name).setSelected(True)

        return True

    def performDelete(self):
        return self.deleteSelectedImageEntries()
*/

void ImagesetVisualMode::slot_selectionChanged()
{
/*
        // if dockWidget is changing the selection, back off
        if self.dockWidget.selectionUnderway:
            return
*/

    auto selectedItems = scene()->selectedItems();
    if (selectedItems.size() == 1)
    {
        ImageEntry* entry = dynamic_cast<ImageEntry*>(selectedItems[0]);
        /*
        if (entry)
            dockWidget.list.scrollToItem(selectedItems[0].listItem);
        */
    }
}
