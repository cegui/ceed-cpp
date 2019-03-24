#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageLabel.h"
#include "src/ui/imageset/ImageOffsetMark.h"
#include "src/ui/MainWindow.h" // for status bar
#include "src/util/Utils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qstatusbar.h"
#include "qdom.h"
#include "qpainter.h"

ImageEntry::ImageEntry(QGraphicsItem* parent)
    : ResizableRectItem(parent)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setVisible(true);

    // Reset to unreachable value
    oldPosition.setX(-10000.0);
    oldPosition.setY(-10000.0);

    label = new ImageLabel(this);
    offset = new ImageOffsetMark(this);

    // List item in the dock widget's ListWidget.
    // This allows fast updates of the list item without looking it up.
    // It is safe to assume that this is None or a valid QListWidgetItem.
/*
        self.listItem = None
*/
}

void ImageEntry::loadFromElement(const QDomElement& xml)
{
    setName(xml.attribute("name", "Unknown"));

    setPos(xml.attribute("xPos", "0").toDouble(), xml.attribute("yPos", "0").toDouble());

    qreal w = xml.attribute("width", "1").toDouble();
    qreal h = xml.attribute("height", "1").toDouble();
    setRect(0.0, 0.0, std::max(1.0, w), std::max(1.0, h));

    setOffsetX(xml.attribute("xOffset", "0").toInt());
    setOffsetY(xml.attribute("yOffset", "0").toInt());

    nativeHorzRes = xml.attribute("nativeHorzRes", "0").toInt();
    nativeVertRes = xml.attribute("nativeVertRes", "0").toInt();
    autoScaled = xml.attribute("autoScaled", "");
}

void ImageEntry::saveToElement(QDomElement& xml)
{
    xml.setTagName("Image");

    xml.setAttribute("name", name());
    xml.setAttribute("xPos", QString::number(static_cast<int>(pos().x())));
    xml.setAttribute("yPos", QString::number(static_cast<int>(pos().y())));
    xml.setAttribute("width", QString::number(static_cast<int>(rect().width())));
    xml.setAttribute("height", QString::number(static_cast<int>(rect().height())));

    // We write none or both
    const int ofsX = offsetX();
    const int ofsY = offsetY();
    if (ofsX || ofsY)
    {
        xml.setAttribute("xPos", QString::number(ofsX));
        xml.setAttribute("yPos", QString::number(ofsY));
    }

    if (nativeHorzRes) xml.setAttribute("nativeHorzRes", QString::number(nativeHorzRes));
    if (nativeVertRes) xml.setAttribute("nativeVertRes", QString::number(nativeVertRes));
    if (!autoScaled.isEmpty()) xml.setAttribute("autoScaled", autoScaled);
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
    return label->toPlainText();
}

void ImageEntry::setName(const QString& newName)
{
    label->setPlainText(newName);
}

int ImageEntry::offsetX() const
{
    return static_cast<int>(-(offset->pos().x() - 0.5));
}

void ImageEntry::setOffsetX(int value)
{
    offset->setX(-static_cast<qreal>(value) + 0.5);
}

int ImageEntry::offsetY() const
{
    return static_cast<int>(-(offset->pos().y() - 0.5));
}

void ImageEntry::setOffsetY(int value)
{
    offset->setY(-static_cast<qreal>(value) + 0.5);
}

void ImageEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    ResizableRectItem::paint(painter, option, widget);

    // To be more visible, we draw yellow rect over the usual dashed double colour rect
    if (isSelected())
    {
        painter->setPen(QColor(255, 255, 0, 255));
        painter->drawRect(rect());
    }
}

QVariant ImageEntry::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool())
        {
            auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
            if (settings->getEntryValue("imageset/visual/overlay_image_labels").toBool())
                label->setVisible(true);

            ImagesetEntry* imagesetEntry = static_cast<ImagesetEntry*>(parentItem());
            if (imagesetEntry->showOffsets())
                offset->setVisible(true);

            setZValue(zValue() + 1);
        }
        else
        {
            if (!isHovered) label->setVisible(false);

            if (!offset->isSelected() && !offset->isHovered())
                offset->setVisible(false);

            setZValue(zValue() - 1);
        }

        updateListItemSelection();
    }
    else if (change == ItemPositionChange)
    {
        if (potentialMove && oldPosition.x() < -9999.0) // FIXME: hack for 'not set' position
        {
            oldPosition = pos();

            // Hide label when moving so user can see edges clearly
            label->setVisible(false);
        }

        auto newPosition = value.toPointF();

        // If, for whatever reason, the loading of the pixmap failed, we don't constrain to the empty null pixmap
        ImagesetEntry* imagesetEntry = static_cast<ImagesetEntry*>(parentItem());
        if (!imagesetEntry->pixmap().isNull())
        {
            auto parentRect = imagesetEntry->boundingRect();
            parentRect.setWidth(parentRect.width() - rect().width());
            parentRect.setHeight(parentRect.height() - rect().height());

            if (!parentRect.contains(newPosition))
            {
                newPosition.setX(std::min(parentRect.right(), std::max(newPosition.x(), parentRect.left())));
                newPosition.setY(std::min(parentRect.bottom(), std::max(newPosition.y(), parentRect.top())));
            }
        }

        // Now round the position to pixels
        newPosition.setX(round(newPosition.x()));
        newPosition.setY(round(newPosition.y()));

        return newPosition;
    }

    return ResizableRectItem::itemChange(change, value);
}

void ImageEntry::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    ResizableRectItem::hoverEnterEvent(event);

    setZValue(zValue() + 1);

    Application* app = qobject_cast<Application*>(qApp);

    auto&& settings = app->getSettings();
    if (settings->getEntryValue("imageset/visual/overlay_image_labels").toBool())
        label->setVisible(true);

    app->getMainWindow()->statusBar()->showMessage(QString("Image: '%1'\t\tXPos: %2, YPos: %3, Width: %4, Height: %5")
                                                   .arg(name()).arg(pos().x()).arg(pos().y()).arg(rect().width()).arg(rect().height()));

    isHovered = true;
}

void ImageEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    isHovered = false;

    qobject_cast<Application*>(qApp)->getMainWindow()->statusBar()->clearMessage();

    if (!isSelected()) label->setVisible(false);

    setZValue(zValue() - 1);

    ResizableRectItem::hoverLeaveEvent(event);
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

// Synchronises the selection in the dock widget's list. This makes sure that when you select
// this item the list sets the selection to this item as well.
void ImageEntry::updateListItemSelection()
{
/*
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
*/
}

/*
    # the image's "real parameters" are properties that directly access Qt's
    # facilities, this is done to make the code cleaner and save a little memory

    nativeRes = property(lambda self: (self.nativeHorzRes, self.nativeVertRes),
                         lambda self, value: self.setNativeRes(value))


    def setNativeRes(self, value):
        # NB: This is just a wrapper to make the property setter lambda work
        self.nativeHorzRes, self.nativeVertRes = value

    def constrainResizeRect(self, rect, oldRect):
        # we simply round the rectangle because we only support "full" pixels

        # NOTE: Imageset as such might support floating point pixels but it's never what you
        #       really want, image quality deteriorates a lot

        rect = QtCore.QRectF(QtCore.QPointF(round(rect.topLeft().x()), round(rect.topLeft().y())),
                             QtCore.QPointF(round(rect.bottomRight().x()), round(rect.bottomRight().y())))

        return super(ImageEntry, self).constrainResizeRect(rect, oldRect)

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
*/
