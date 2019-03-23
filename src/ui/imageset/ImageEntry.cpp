#include "src/ui/imageset/ImageEntry.h"

ImageEntry::ImageEntry()
{

}

/*

class ImageEntry(resizable.ResizableRectItem):
    """
    """

    # the image's "real parameters" are properties that directly access Qt's
    # facilities, this is done to make the code cleaner and save a little memory

    name = property(lambda self: self.label.toPlainText(),
                    lambda self, value: self.label.setPlainText(value))

    xpos = property(lambda self: int(self.pos().x()),
                    lambda self, value: self.setPos(value, self.pos().y()))
    ypos = property(lambda self: int(self.pos().y()),
                    lambda self, value: self.setPos(self.pos().x(), value))
    width = property(lambda self: int(self.rect().width()),
                     lambda self, value: self.setRect(QtCore.QRectF(0, 0, max(1, value), self.height)))
    height = property(lambda self: int(self.rect().height()),
                      lambda self, value: self.setRect(QtCore.QRectF(0, 0, self.width, max(1, value))))

    xoffset = property(lambda self: int(-(self.offset.pos().x() - 0.5)),
                       lambda self, value: self.offset.setX(-float(value) + 0.5))
    yoffset = property(lambda self: int(-(self.offset.pos().y() - 0.5)),
                       lambda self, value: self.offset.setY(-float(value) + 0.5))

    nativeRes = property(lambda self: (self.nativeHorzRes, self.nativeVertRes),
                         lambda self, value: self.setNativeRes(value))

    def __init__(self, imagesetEntry):
        super(ImageEntry, self).__init__(imagesetEntry)

        self.nativeHorzRes = 0
        self.nativeVertRes = 0
        self.autoScaled = ""

        self.imagesetEntry = imagesetEntry

        self.setAcceptsHoverEvents(True)
        self.isHovered = False

        # used for undo
        self.potentialMove = False
        self.oldPosition = None
        self.resized = False

        self.setFlags(QtGui.QGraphicsItem.ItemIsMovable |
                      QtGui.QGraphicsItem.ItemIsSelectable |
                      QtGui.QGraphicsItem.ItemSendsGeometryChanges)

        self.setVisible(True)

        self.label = ImageLabel(self)
        self.offset = ImageOffset(self)

        # list item in the dock widget's ListWidget
        # this allows fast updates of the list item without looking it up
        # It is save to assume that this is None or a valid QListWidgetItem
        self.listItem = None

    def constrainResizeRect(self, rect, oldRect):
        # we simply round the rectangle because we only support "full" pixels

        # NOTE: Imageset as such might support floating point pixels but it's never what you
        #       really want, image quality deteriorates a lot

        rect = QtCore.QRectF(QtCore.QPointF(round(rect.topLeft().x()), round(rect.topLeft().y())),
                             QtCore.QPointF(round(rect.bottomRight().x()), round(rect.bottomRight().y())))

        return super(ImageEntry, self).constrainResizeRect(rect, oldRect)

    def loadFromElement(self, element):
        self.name = element.get("name", "Unknown")

        self.xpos = int(element.get("xPos", 0))
        self.ypos = int(element.get("yPos", 0))
        self.width = int(element.get("width", 1))
        self.height = int(element.get("height", 1))

        self.xoffset = int(element.get("xOffset", 0))
        self.yoffset = int(element.get("yOffset", 0))

        self.nativeHorzRes = int(element.get("nativeHorzRes", 0))
        self.nativeVertRes = int(element.get("nativeVertRes", 0))
        self.autoScaled = element.get("autoScaled", "")

    def saveToElement(self):
        ret = ElementTree.Element("Image")

        ret.set("name", self.name)

        ret.set("xPos", str(self.xpos))
        ret.set("yPos", str(self.ypos))
        ret.set("width", str(self.width))
        ret.set("height", str(self.height))

        # we write none or both
        if self.xoffset != 0 or self.yoffset != 0:
            ret.set("xOffset", str(self.xoffset))
            ret.set("yOffset", str(self.yoffset))

        if self.nativeHorzRes != 0:
            ret.set("nativeHorzRes", str(self.nativeHorzRes))

        if self.nativeVertRes != 0:
            ret.set("nativeVertRes", str(self.nativeVertRes))

        if self.autoScaled != "":
            ret.set("autoScaled", self.autoScaled)

        return ret

    def getPixmap(self):
        """Creates and returns a pixmap containing what's in the underlying image in the rectangle
        that this ImageEntry has set.

        This is mostly used for preview thumbnails in the dock widget.
        """
        return self.imagesetEntry.pixmap().copy(int(self.pos().x()), int(self.pos().y()),
                                                int(self.rect().width()), int(self.rect().height()))

    def updateListItem(self):
        """Updates the list item associated with this image entry in the dock widget
        """

        if not self.listItem:
            return

        self.listItem.setText(self.name)

        previewWidth = 24
        previewHeight = 24

        preview = QtGui.QPixmap(previewWidth, previewHeight)
        painter = QtGui.QPainter(preview)
        painter.setBrush(qtwidgets.getCheckerboardBrush())
        painter.drawRect(0, 0, previewWidth, previewHeight)
        scaledPixmap = self.getPixmap().scaled(QtCore.QSize(previewWidth, previewHeight),
                                               QtCore.Qt.KeepAspectRatio, QtCore.Qt.SmoothTransformation)
        painter.drawPixmap((previewWidth - scaledPixmap.width()) / 2,
                           (previewHeight - scaledPixmap.height()) / 2,
                           scaledPixmap)
        painter.end()

        self.listItem.setIcon(QtGui.QIcon(preview))

    def updateListItemSelection(self):
        """Synchronises the selection in the dock widget's list. This makes sure that when you select
        this item the list sets the selection to this item as well.
        """

        if not self.listItem:
            return

        dockWidget = self.listItem.dockWidget

        # the dock widget itself is performing a selection, we shall not interfere
        if dockWidget.selectionUnderway:
            return

        dockWidget.selectionSynchronisationUnderway = True

        if self.isSelected() or self.isAnyHandleSelected() or self.offset.isSelected():
            self.listItem.setSelected(True)
        else:
            self.listItem.setSelected(False)

        dockWidget.selectionSynchronisationUnderway = False

    def updateDockWidget(self):
        """If we are selected in the dock widget, this updates the property box
        """

        self.updateListItem()

        if not self.listItem:
            return

        dockWidget = self.listItem.dockWidget
        if dockWidget.activeImageEntry == self:
            dockWidget.refreshActiveImageEntry()

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemSelectedHasChanged:
            if value:
                if settings.getEntry("imageset/visual/overlay_image_labels").value:
                    self.label.setVisible(True)

                if self.imagesetEntry.showOffsets:
                    self.offset.setVisible(True)

                self.setZValue(self.zValue() + 1)
            else:
                if not self.isHovered:
                    self.label.setVisible(False)

                if not self.offset.isSelected() and not self.offset.isHovered:
                    self.offset.setVisible(False)

                self.setZValue(self.zValue() - 1)

            self.updateListItemSelection()

        elif change == QtGui.QGraphicsItem.ItemPositionChange:
            if self.potentialMove and not self.oldPosition:
                self.oldPosition = self.pos()
                # hide label when moving so user can see edges clearly
                self.label.setVisible(False)

            newPosition = value

            # if, for whatever reason, the loading of the pixmap failed,
            # we don't constrain to the empty null pixmap

            # only constrain when the pixmap is valid
            if not self.imagesetEntry.pixmap().isNull():

                rect = self.imagesetEntry.boundingRect()
                rect.setWidth(rect.width() - self.rect().width())
                rect.setHeight(rect.height() - self.rect().height())

                if not rect.contains(newPosition):
                    newPosition.setX(min(rect.right(), max(newPosition.x(), rect.left())))
                    newPosition.setY(min(rect.bottom(), max(newPosition.y(), rect.top())))

            # now round the position to pixels
            newPosition.setX(round(newPosition.x()))
            newPosition.setY(round(newPosition.y()))

            return newPosition

        return super(ImageEntry, self).itemChange(change, value)

    def notifyResizeStarted(self):
        super(ImageEntry, self).notifyResizeStarted()

        # hide label when resizing so user can see edges clearly
        self.label.setVisible(False)

    def notifyResizeFinished(self, newPos, newRect):
        super(ImageEntry, self).notifyResizeFinished(newPos, newRect)

        if self.mouseOver and settings.getEntry("imageset/visual/overlay_image_labels").value:
            # if mouse is over we show the label again when resizing finishes
            self.label.setVisible(True)

        # mark as resized so we can pick it up in VisualEditing.mouseReleaseEvent
        self.resized = True

    def hoverEnterEvent(self, event):
        super(ImageEntry, self).hoverEnterEvent(event)

        self.setZValue(self.zValue() + 1)

        if settings.getEntry("imageset/visual/overlay_image_labels").value:
            self.label.setVisible(True)

        mainwindow.MainWindow.instance.statusBar().showMessage("Image: '%s'\t\tXPos: %i, YPos: %i, Width: %i, Height: %i" %
                                                               (self.name, self.pos().x(), self.pos().y(), self.rect().width(), self.rect().height()))

        self.isHovered = True

    def hoverLeaveEvent(self, event):
        mainwindow.MainWindow.instance.statusBar().clearMessage()

        self.isHovered = False

        if not self.isSelected():
            self.label.setVisible(False)

        self.setZValue(self.zValue() - 1)

        super(ImageEntry, self).hoverLeaveEvent(event)

    def paint(self, painter, option, widget):
        super(ImageEntry, self).paint(painter, option, widget)

        # to be more visible, we draw yellow rect over the usual dashed double colour rect
        if self.isSelected():
            painter.setPen(QtGui.QColor(255, 255, 0, 255))
            painter.drawRect(self.rect())

    def setNativeRes(self, value):
        # NB: This is just a wrapper to make the property setter lambda work
        self.nativeHorzRes, self.nativeVertRes = value
*/
