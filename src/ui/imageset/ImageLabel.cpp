#include "src/ui/imageset/ImageLabel.h"

ImageLabel::ImageLabel()
{

}

/*
class ImageLabel(QtGui.QGraphicsTextItem):
    """
    """

    def __init__(self, imageEntry):
        super(ImageLabel, self).__init__(imageEntry)

        self.imageEntry = imageEntry

        self.setFlags(QtGui.QGraphicsItem.ItemIgnoresTransformations)
        self.setOpacity(0.8)

        self.setPlainText("Unknown")

        # we make the label a lot more transparent when mouse is over it to make it easier
        # to work around the top edge of the image
        self.setAcceptHoverEvents(True)
        # the default opacity (when mouse is not over the label)
        self.setOpacity(0.8)

        # be invisible by default and wait for hover/selection events
        self.setVisible(False)

    def paint(self, painter, option, widget):
        palette = QtGui.QApplication.palette()

        painter.fillRect(self.boundingRect(), palette.color(QtGui.QPalette.Normal, QtGui.QPalette.Base))
        painter.drawRect(self.boundingRect())

        super(ImageLabel, self).paint(painter, option, widget)

    def hoverEnterEvent(self, event):
        super(ImageLabel, self).hoverEnterEvent(event)

        self.setOpacity(0.2)

    def hoverLeaveEvent(self, event):
        self.setOpacity(0.8)

        super(ImageLabel, self).hoverLeaveEvent(event)
*/
