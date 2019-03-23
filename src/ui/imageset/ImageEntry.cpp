#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/util/Utils.h"
#include "qdom.h"
#include "qpainter.h"

ImageEntry::ImageEntry(QGraphicsItem* parent)
    : ResizableRectItem(parent)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setVisible(true);

    // Reset to unreachable value
    oldPosition.setX(-10000);
    oldPosition.setY(-10000);

/*
        self.label = ImageLabel(self)
        self.offset = ImageOffset(self)

        # list item in the dock widget's ListWidget
        # this allows fast updates of the list item without looking it up
        # It is safe to assume that this is None or a valid QListWidgetItem
        self.listItem = None
*/
}

void ImageEntry::loadFromElement(const QDomElement& xml)
{
/*
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
*/
}

void ImageEntry::saveToElement(QDomElement& xml)
{
    xml.setTagName("Image");
/*
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
*/
}

// If we are selected in the dock widget, this updates the property box
void ImageEntry::updateDockWidget()
{
    updateListItem();
/*
        if not self.listItem:
            return

        dockWidget = self.listItem.dockWidget
        if dockWidget.activeImageEntry == self:
            dockWidget.refreshActiveImageEntry()
*/
}

// Updates the list item associated with this image entry in the dock widget
void ImageEntry::updateListItem()
{
/*
        if not self.listItem:
            return

        self.listItem.setText(self.name)
*/
    constexpr int previewWidth = 24;
    constexpr int previewHeight = 24;

    QPixmap preview(previewWidth, previewHeight);
    QPainter painter(&preview);
    painter.setBrush(Utils::getCheckerboardBrush());
    painter.drawRect(0, 0, previewWidth, previewHeight);
    QPixmap scaledPixmap = getPixmap().scaled(QSize(previewWidth, previewHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap((previewWidth - scaledPixmap.width()) / 2, (previewHeight - scaledPixmap.height()) / 2, scaledPixmap);
    painter.end();

/*
        self.listItem.setIcon(QtGui.QIcon(preview))
*/
}

QString ImageEntry::name() const
{
/*
    label.toPlainText()
*/
    return "";
}

void ImageEntry::setName(const QString &newName)
{
/*
    label.setPlainText(value)
*/
}

// Creates and returns a pixmap containing what's in the underlying image in the rectangle
// that this ImageEntry has set. This is mostly used for preview thumbnails in the dock widget.
QPixmap ImageEntry::getPixmap()
{
    ImagesetEntry* imagesetEntry = static_cast<ImagesetEntry*>(parentItem());
    return imagesetEntry->pixmap().copy(
                static_cast<int>(pos().x()),
                static_cast<int>(pos().y()),
                static_cast<int>(rect().width()),
                static_cast<int>(rect().height()));
}

/*
    # the image's "real parameters" are properties that directly access Qt's
    # facilities, this is done to make the code cleaner and save a little memory

    name = property(lambda self: self.,
                    lambda self, value: self.)

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


    def constrainResizeRect(self, rect, oldRect):
        # we simply round the rectangle because we only support "full" pixels

        # NOTE: Imageset as such might support floating point pixels but it's never what you
        #       really want, image quality deteriorates a lot

        rect = QtCore.QRectF(QtCore.QPointF(round(rect.topLeft().x()), round(rect.topLeft().y())),
                             QtCore.QPointF(round(rect.bottomRight().x()), round(rect.bottomRight().y())))

        return super(ImageEntry, self).constrainResizeRect(rect, oldRect)

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
