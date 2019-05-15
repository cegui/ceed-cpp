#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageLabel.h"
#include "src/ui/imageset/ImageOffsetMark.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/ui/MainWindow.h" // for status bar
#include "src/util/Utils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qstatusbar.h"
#include "qdom.h"
#include "qpainter.h"
#include "qlistwidget.h"

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
}

ImageEntry::~ImageEntry()
{
    if (listItem)
    {
        listItem->setData(Qt::UserRole + 2, QVariant());
        listItem = nullptr;
        //???or delete list item?
    }
}

// We simply round the rectangle because we only support "full" pixels
// NOTE: Imageset as such might support floating point pixels but it's never what you really want, image quality deteriorates a lot
QRectF ImageEntry::constrainResizeRect(QRectF rect, QRectF oldRect)
{
    rect = QRectF(QPointF(round(rect.topLeft().x()), round(rect.topLeft().y())),
                  QPointF(round(rect.bottomRight().x()), round(rect.bottomRight().y())));
    return ResizableRectItem::constrainResizeRect(rect, oldRect);
}

void ImageEntry::notifyResizeStarted()
{
    ResizableRectItem::notifyResizeStarted();

    // Hide label when resizing so user can see edges clearly
    label->setVisible(false);
}

void ImageEntry::notifyResizeFinished(QPointF newPos, QRectF newRect)
{
    ResizableRectItem::notifyResizeFinished(newPos, newRect);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    if (_mouseOver && settings->getEntryValue("imageset/visual/overlay_image_labels").toBool())
    {
        // If mouse is over we show the label again when resizing finishes
        label->setVisible(true);
    }

    // Mark as resized so we can pick it up in VisualEditing.mouseReleaseEvent
    resized = true;
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
    if (!listItem) return;

    updateListItem();

    // TODO: redesign?
    auto dockWidget = listItem->data(Qt::UserRole + 1).value<ImagesetEditorDockWidget*>();
    if (dockWidget->getActiveImageEntry() == this)
        dockWidget->refreshActiveImageEntry();
}

// Updates the list item associated with this image entry in the dock widget
void ImageEntry::updateListItem()
{
    if (!listItem) return;

    listItem->setText(name());

    constexpr int previewWidth = 24;
    constexpr int previewHeight = 24;

    QPixmap preview(previewWidth, previewHeight);
    QPainter painter(&preview);
    painter.setBrush(Utils::getCheckerboardBrush());
    painter.drawRect(0, 0, previewWidth, previewHeight);
    QPixmap scaledPixmap = getPixmap().scaled(QSize(previewWidth, previewHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap((previewWidth - scaledPixmap.width()) / 2, (previewHeight - scaledPixmap.height()) / 2, scaledPixmap);
    painter.end();

    listItem->setIcon(QIcon(preview));
}

void ImageEntry::showLabel(bool show)
{
    label->setVisible(show);
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

// Python legacy. Some code used python properties with implicit setter & getter like this:
// setattr(imageEntry, self.propertyName, self.oldValue)
void ImageEntry::setProperty(const QString& name, const QVariant& value)
{
    if (name == "xpos") setPos(value.toReal(), pos().y());
    else if (name == "ypos") setPos(pos().x(), value.toReal());
    else if (name == "width") setRect(QRectF(0.0, 0.0, std::max(1.0, value.toReal()), rect().height()));
    else if (name == "height") setRect(QRectF(0.0, 0.0, rect().width(), std::max(1.0, value.toReal())));
    else if (name == "xoffset") setOffsetX(value.toInt());
    else if (name == "yoffset") setOffsetY(value.toInt());
    else if (name == "autoScaled") autoScaled = value.toString();
    else if (name == "nativeRes")
    {
        QPoint val = value.toPoint();
        nativeHorzRes = val.x();
        nativeVertRes = val.y();
    }
    else assert(false && "Unsupported property");
}

QVariant ImageEntry::getProperty(const QString& name)
{
    if (name == "xpos") return pos().x();
    else if (name == "ypos") return pos().y();
    else if (name == "width") return rect().width();
    else if (name == "height") return rect().height();
    else if (name == "xoffset") return offsetX();
    else if (name == "yoffset") return offsetY();
    else if (name == "autoScaled") return autoScaled;
    else if (name == "nativeRes") return QPoint(nativeHorzRes, nativeVertRes);
    else
    {
        assert(false && "Unsupported property");
        return QVariant();
    }
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
            if (!_isHovered) label->setVisible(false);

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

    _isHovered = true;
}

void ImageEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _isHovered = false;

    qobject_cast<Application*>(qApp)->getMainWindow()->statusBar()->clearMessage();

    if (!isSelected()) label->setVisible(false);

    setZValue(zValue() - 1);

    ResizableRectItem::hoverLeaveEvent(event);
}

void ImageEntry::onPotentialMove(bool move)
{
    potentialMove = move;
    if (!move) resized = false;

    // Reset to unreachable value
    oldPosition.setX(-10000.0);
    oldPosition.setY(-10000.0);
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
    if (!listItem) return;

    auto dockWidget = listItem->data(Qt::UserRole + 1).value<ImagesetEditorDockWidget*>();

    // The dock widget itself is performing a selection, we shall not interfere
    if (dockWidget->isSelectionUnderway()) return;

    dockWidget->setSelectionSynchronizationUnderway(true);
    listItem->setSelected(isSelected() || isAnyHandleSelected() || offset->isSelected());
    dockWidget->setSelectionSynchronizationUnderway(false);
}
