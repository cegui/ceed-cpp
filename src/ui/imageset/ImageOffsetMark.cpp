#include "src/ui/imageset/ImageOffsetMark.h"

ImageOffsetMark::ImageOffsetMark()
{

}

/*

class ImageOffset(QtGui.QGraphicsPixmapItem):
    """
    """

    def __init__(self, imageEntry):
        super(ImageOffset, self).__init__(imageEntry)

        self.imageEntry = imageEntry

        self.setFlags(QtGui.QGraphicsItem.ItemIsMovable |
                      QtGui.QGraphicsItem.ItemIsSelectable |
                      QtGui.QGraphicsItem.ItemIgnoresTransformations |
                      QtGui.QGraphicsItem.ItemSendsGeometryChanges)

        self.setCursor(QtCore.Qt.OpenHandCursor)

        self.setPixmap(QtGui.QPixmap("icons/imageset_editing/offset_crosshair.png"))
        # the crosshair pixmap is 15x15, (7, 7) is the centre pixel of it,
        # we want that to be the (0, 0) point of the crosshair
        self.setOffset(-7, -7)
        # always show this above the label (which has ZValue = 0)
        self.setZValue(1)

        self.setAcceptHoverEvents(True)
        # internal attribute to help decide when to hide/show the offset crosshair
        self.isHovered = False

        # used for undo
        self.potentialMove = False
        self.oldPosition = None

        # by default Qt considers parts of the image with alpha = 0 not part of the image,
        # that would make it very hard to move the crosshair, we consider the whole
        # bounding rectangle to be part of the image
        self.setShapeMode(QtGui.QGraphicsPixmapItem.BoundingRectShape)
        self.setVisible(False)

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemPositionChange:
            if self.potentialMove and not self.oldPosition:
                self.oldPosition = self.pos()

            newPosition = value

            # now round the position to pixels
            newPosition.setX(round(newPosition.x() - 0.5) + 0.5)
            newPosition.setY(round(newPosition.y() - 0.5) + 0.5)

            return newPosition

        elif change == QtGui.QGraphicsItem.ItemSelectedChange:
            if not value:
                if not self.imageEntry.isSelected():
                    self.setVisible(False)
            else:
                self.setVisible(True)

        return super(ImageOffset, self).itemChange(change, value)

    def hoverEnterEvent(self, event):
        super(ImageOffset, self).hoverEnterEvent(event)

        self.isHovered = True

    def hoverLeaveEvent(self, event):
        self.isHovered = False

        super(ImageOffset, self).hoverLeaveEvent(event)
*/
