#include "src/ui/imageset/ImagesetEntry.h"
#include "qdom.h"

ImagesetEntry::ImagesetEntry(QGraphicsItem* parent)
    : QGraphicsPixmapItem(/*parent*/)
{
/*
        self.imageFile = ""

        self.setShapeMode(QtGui.QGraphicsPixmapItem.BoundingRectShape)
        self.setCursor(QtCore.Qt.ArrowCursor)

        self.visual = visual
        self.imageEntries = []

        self.showOffsets = False

        self.transparencyBackground = QtGui.QGraphicsRectItem()
        self.transparencyBackground.setParentItem(self)
        self.transparencyBackground.setFlags(QtGui.QGraphicsItem.ItemStacksBehindParent)

        self.transparencyBackground.setBrush(qtwidgets.getCheckerboardBrush())
        self.transparencyBackground.setPen(QtGui.QPen(QtGui.QColor(QtCore.Qt.transparent)))

        self.imageMonitor = None
        self.displayingReloadAlert = False
*/
}

void ImagesetEntry::loadFromElement(const QDomElement& xml)
{
    _name = xml.attribute("name", "Unknown");
/*

    self.loadImage(xml.attribute("imagefile", ""))
*/

    nativeHorzRes = xml.attribute("nativeHorzRes", "800").toInt();
    nativeVertRes = xml.attribute("nativeVertRes", "600").toInt();

    autoScaled = xml.attribute("autoScaled", "false");

    auto xmlImage = xml.firstChildElement("Image");
    while (!xmlImage.isNull())
    {
/*
        image = ImageEntry(self)
        image.loadFromElement(imageElement)
        self.imageEntries.append(image)
*/
        xmlImage = xmlImage.nextSiblingElement("Image");
    }
}

/*
    def getImageEntry(self, name):
        for image in self.imageEntries:
            if image.name == name:
                return image

        assert(False)
        return None

    def slot_imageChangedByExternalProgram(self):
        """Monitor the image with a QFilesystemWatcher, ask user to reload
        if changes to the file were made."""

        if not self.displayingReloadAlert:
            self.displayingReloadAlert = True
            ret = QtGui.QMessageBox.question(self.visual.tabbedEditor.mainWindow,
                                             "Underlying image '%s' has been modified externally!" % (self.imageFile),
                                             "The file has been modified outside the CEGUI Unified Editor.\n\nReload the file?\n\nIf you select Yes, UNDO HISTORY MIGHT BE PARTIALLY BROKEN UNLESS THE NEW SIZE IS THE SAME OR LARGER THAN THE OLD!",
                                             QtGui.QMessageBox.No | QtGui.QMessageBox.Yes,
                                             QtGui.QMessageBox.No) # defaulting to No is safer IMO

            if ret == QtGui.QMessageBox.Yes:
                self.loadImage(self.imageFile)

            elif ret == QtGui.QMessageBox.No:
                pass

            else:
                # how did we get here?
                assert(False)

            self.displayingReloadAlert = False

    def loadImage(self, relativeImagePath):
        """
        Replaces the underlying image (if any is loaded) to the image on given relative path

        Relative path is relative to the directory where the .imageset file resides
        (which is usually your project's imageset resource group path)
        """

        # If imageMonitor is null, then no images are being watched or the
        # editor is first being opened up
        # Otherwise, the image is being changed or switched, and the monitor
        # should update itself accordingly
        if self.imageMonitor != None:
            self.imageMonitor.removePath(self.getAbsoluteImageFile())

        self.imageFile = relativeImagePath
        self.setPixmap(QtGui.QPixmap(self.getAbsoluteImageFile()))
        self.transparencyBackground.setRect(self.boundingRect())

        # go over all image entries and set their position to force them to be constrained
        # to the new pixmap's dimensions
        for imageEntry in self.imageEntries:
            imageEntry.setPos(imageEntry.pos())
            imageEntry.updateDockWidget()

        self.visual.refreshSceneRect()

        # If imageMonitor is null, allocate and watch the loaded file
        if self.imageMonitor == None:
            self.imageMonitor = QtCore.QFileSystemWatcher(None)
            self.imageMonitor.fileChanged.connect(self.slot_imageChangedByExternalProgram)
        self.imageMonitor.addPath(self.getAbsoluteImageFile())

    def getAbsoluteImageFile(self):
        """Returns an absolute (OS specific!) path of the underlying image
        """

        return os.path.join(os.path.dirname(self.visual.tabbedEditor.filePath), self.imageFile)

    def convertToRelativeImageFile(self, absoluteImageFile):
        """Converts given absolute underlying image path to relative path (relative to the directory where
        the .imageset file resides
        """

        return os.path.normpath(os.path.relpath(absoluteImageFile, os.path.dirname(self.visual.tabbedEditor.filePath)))

    def saveToElement(self):
        ret = ElementTree.Element("Imageset")

        ret.set("version", "2")

        ret.set("name", self.name)
        ret.set("imagefile", self.imageFile)

        ret.set("nativeHorzRes", str(self.nativeHorzRes))
        ret.set("nativeVertRes", str(self.nativeVertRes))
        ret.set("autoScaled", self.autoScaled)

        for image in self.imageEntries:
            ret.append(image.saveToElement())

        return ret
*/
