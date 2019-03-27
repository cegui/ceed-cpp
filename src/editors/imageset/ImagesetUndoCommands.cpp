#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImageOffsetMark.h"

ImagesetMoveCommand::ImagesetMoveCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    for (const auto& rec : _imageRecords)
    {
        auto positionDelta = rec.oldPos - rec.newPos;
        auto delta = sqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y());
        if (delta > biggestDelta) biggestDelta = delta;
    }

    refreshText();
}

void ImagesetMoveCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->setPos(rec.oldPos);
        image->updateDockWidget();
    }
}

void ImagesetMoveCommand::redo()
{
    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->setPos(rec.newPos);
        image->updateDockWidget();
    }

    QUndoCommand::redo();
}

bool ImagesetMoveCommand::mergeWith(const QUndoCommand* other)
{
    const ImagesetMoveCommand* otherCmd = dynamic_cast<const ImagesetMoveCommand*>(other);
    if (!otherCmd) return false;

    if (_imageRecords.size() != otherCmd->_imageRecords.size()) return false;

    // TODO: 50 used just for testing!
    auto combinedBiggestDelta = biggestDelta + otherCmd->biggestDelta;
    if (combinedBiggestDelta >= 50) return false;

    QStringList names;
    for (const auto& rec : _imageRecords)
        names.push_back(rec.name);

    for (const auto& rec : otherCmd->_imageRecords)
        if (!names.contains(rec.name)) return false;

    // The same set of images, can merge

    for (auto& rec : _imageRecords)
    {
        const QString& name = rec.name;
        auto it = std::find_if(otherCmd->_imageRecords.begin(), otherCmd->_imageRecords.end(), [&name](const Record& otherRec)
        {
            return otherRec.name == name;
        });
        assert(it != otherCmd->_imageRecords.end());

        rec.newPos = it->newPos;
    }

    biggestDelta = combinedBiggestDelta;
    refreshText();
    return true;
}

void ImagesetMoveCommand::refreshText()
{
    if (_imageRecords.size() == 1)
        setText(QString("Move '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Move %1 images'").arg(_imageRecords.size()));
}

//---------------------------------------------------------------------

ImagesetGeometryChangeCommand::ImagesetGeometryChangeCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    for (const auto& rec : _imageRecords)
    {
        auto positionDelta = rec.oldPos - rec.newPos;
        auto delta = sqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y());
        if (delta > biggestMoveDelta) biggestMoveDelta = delta;

        auto resizeDelta = rec.oldRect.bottomRight() - rec.newRect.bottomRight();
        delta = sqrt(resizeDelta.x() * resizeDelta.x() + resizeDelta.y() * resizeDelta.y());
        if (delta > biggestResizeDelta) biggestResizeDelta = delta;
    }

    refreshText();
}

void ImagesetGeometryChangeCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->setPos(rec.oldPos);
        image->setRect(rec.oldRect);
        image->updateDockWidget();
    }
}

void ImagesetGeometryChangeCommand::redo()
{
    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->setPos(rec.newPos);
        image->setRect(rec.newRect);
        image->updateDockWidget();
    }

    QUndoCommand::redo();
}

bool ImagesetGeometryChangeCommand::mergeWith(const QUndoCommand* other)
{
    const ImagesetGeometryChangeCommand* otherCmd = dynamic_cast<const ImagesetGeometryChangeCommand*>(other);
    if (!otherCmd) return false;

    if (_imageRecords.size() != otherCmd->_imageRecords.size()) return false;

    // TODO: 50 used just for testing!
    auto combinedBiggestMoveDelta = biggestMoveDelta + otherCmd->biggestMoveDelta;
    if (combinedBiggestMoveDelta >= 50) return false;

    // TODO: 20 used just for testing!
    auto combinedBiggestResizeDelta = biggestResizeDelta + otherCmd->biggestResizeDelta;
    if (combinedBiggestResizeDelta >= 20) return false;

    QStringList names;
    for (const auto& rec : _imageRecords)
        names.push_back(rec.name);

    for (const auto& rec : otherCmd->_imageRecords)
        if (!names.contains(rec.name)) return false;

    // The same set of images, can merge

    for (auto& rec : _imageRecords)
    {
        const QString& name = rec.name;
        auto it = std::find_if(otherCmd->_imageRecords.begin(), otherCmd->_imageRecords.end(), [&name](const Record& otherRec)
        {
            return otherRec.name == name;
        });
        assert(it != otherCmd->_imageRecords.end());

        rec.newPos = it->newPos;
        rec.newRect = it->newRect;
    }

    biggestMoveDelta = combinedBiggestMoveDelta;
    biggestResizeDelta = combinedBiggestResizeDelta;
    refreshText();
    return true;
}

void ImagesetGeometryChangeCommand::refreshText()
{
    if (_imageRecords.size() == 1)
        setText(QString("Change geometry of '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Change geometry of %1 images'").arg(_imageRecords.size()));
}

//---------------------------------------------------------------------

ImagesetOffsetMoveCommand::ImagesetOffsetMoveCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    for (const auto& rec : _imageRecords)
    {
        auto positionDelta = rec.oldPos - rec.newPos;
        auto delta = sqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y());
        if (delta > biggestDelta) biggestDelta = delta;
    }

    refreshText();
}

void ImagesetOffsetMoveCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->getOffsetMark()->setPos(rec.oldPos);
    }
}

void ImagesetOffsetMoveCommand::redo()
{
    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        image->getOffsetMark()->setPos(rec.newPos);
    }

    QUndoCommand::redo();
}

bool ImagesetOffsetMoveCommand::mergeWith(const QUndoCommand* other)
{
    const ImagesetOffsetMoveCommand* otherCmd = dynamic_cast<const ImagesetOffsetMoveCommand*>(other);
    if (!otherCmd) return false;

    if (_imageRecords.size() != otherCmd->_imageRecords.size()) return false;

    // TODO: 10 used just for testing!
    auto combinedBiggestDelta = biggestDelta + otherCmd->biggestDelta;
    if (combinedBiggestDelta >= 10) return false;

    QStringList names;
    for (const auto& rec : _imageRecords)
        names.push_back(rec.name);

    for (const auto& rec : otherCmd->_imageRecords)
        if (!names.contains(rec.name)) return false;

    // The same set of images, can merge

    for (auto& rec : _imageRecords)
    {
        const QString& name = rec.name;
        auto it = std::find_if(otherCmd->_imageRecords.begin(), otherCmd->_imageRecords.end(), [&name](const Record& otherRec)
        {
            return otherRec.name == name;
        });
        assert(it != otherCmd->_imageRecords.end());

        rec.newPos = it->newPos;
    }

    biggestDelta = combinedBiggestDelta;
    refreshText();
    return true;
}

void ImagesetOffsetMoveCommand::refreshText()
{
    if (_imageRecords.size() == 1)
        setText(QString("Move offset of '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Move offset of %1 images'").arg(_imageRecords.size()));
}

//---------------------------------------------------------------------

/*
class RenameCommand(commands.UndoCommand):
    """Changes name of one image (always just one image!)
    """

    def __init__(self, visual, oldName, newName):
        super(RenameCommand, self).__init__()

        self.visual = visual

        self.oldName = oldName
        self.newName = newName

        self.refreshText()

    def refreshText(self):
        self.setText("Rename '%s' to '%s'" % (self.oldName, self.newName))

    def id(self):
        return idbase + 4

    def mergeWith(self, cmd):
        if self.newName == cmd.oldName:
            # if our old newName is the same as oldName of the command that
            # comes after this command, we can merge them
            self.newName = cmd.newName
            self.refreshText()

            return True

        return False

    def undo(self):
        super(RenameCommand, self).undo()

        imageEntry = self.visual.imagesetEntry.getImageEntry(self.newName)
        imageEntry.name = self.oldName
        imageEntry.updateListItem()

    def redo(self):
        imageEntry = self.visual.imagesetEntry.getImageEntry(self.oldName)
        imageEntry.name = self.newName
        imageEntry.updateListItem()

        super(RenameCommand, self).redo()

class PropertyEditCommand(commands.UndoCommand):
    """Changes one property of the image.

    We do this separately from Move, OffsetMove, etc commands because we want to
    always merge in this case.
    """

    def __init__(self, visual, imageName, propertyName, oldValue, newValue):
        super(PropertyEditCommand, self).__init__()

        self.visual = visual
        self.imageName = imageName
        self.propertyName = propertyName
        self.oldValue = oldValue
        self.newValue = newValue

        self.refreshText()

    def refreshText(self):
        self.setText("Change %s of '%s' to '%s'" % (self.propertyName, self.imageName, self.newValue))

    def id(self):
        return idbase + 5

    def mergeWith(self, cmd):
        if self.imageName == cmd.imageName and self.propertyName == cmd.propertyName:
            self.newValue = cmd.newValue

            self.refreshText()

            return True

        return False

    def undo(self):
        super(PropertyEditCommand, self).undo()

        imageEntry = self.visual.imagesetEntry.getImageEntry(self.imageName)
        setattr(imageEntry, self.propertyName, self.oldValue)
        imageEntry.updateDockWidget()

    def redo(self):
        imageEntry = self.visual.imagesetEntry.getImageEntry(self.imageName)
        setattr(imageEntry, self.propertyName, self.newValue)
        imageEntry.updateDockWidget()

        super(PropertyEditCommand, self).redo()

class CreateCommand(commands.UndoCommand):
    """Creates one image with given parameters
    """

    def __init__(self, visual, name, xpos, ypos, width, height, xoffset, yoffset):
        super(CreateCommand, self).__init__()

        self.visual = visual

        self.name = name

        self.xpos = xpos
        self.ypos = ypos
        self.width = width
        self.height = height
        self.xoffset = xoffset
        self.yoffset = yoffset

        self.setText("Create '%s'" % (self.name))

    def id(self):
        return idbase + 6

    def undo(self):
        super(CreateCommand, self).undo()

        image = self.visual.imagesetEntry.getImageEntry(self.name)
        self.visual.imagesetEntry.imageEntries.remove(image)

        image.listItem.imageEntry = None
        image.listItem = None

        image.setParentItem(None)
        self.visual.scene().removeItem(image)

        self.visual.dockWidget.refresh()

    def redo(self):
        image = elements.ImageEntry(self.visual.imagesetEntry)
        self.visual.imagesetEntry.imageEntries.append(image)

        image.name = self.name
        image.xpos = self.xpos
        image.ypos = self.ypos
        image.width = self.width
        image.height = self.height
        image.xoffset = self.xoffset
        image.yoffset = self.yoffset
        self.visual.dockWidget.refresh()

        super(CreateCommand, self).redo()

class DeleteCommand(commands.UndoCommand):
    """Deletes given image entries
    """

    def __init__(self, visual, imageNames, oldPositions, oldRects, oldOffsets):
        super(DeleteCommand, self).__init__()

        self.visual = visual

        self.imageNames = imageNames

        self.oldPositions = oldPositions
        self.oldRects = oldRects
        self.oldOffsets = oldOffsets

        if len(self.imageNames) == 1:
            self.setText("Delete '%s'" % (self.imageNames[0]))
        else:
            self.setText("Delete %i images" % (len(self.imageNames)))

    def id(self):
        return idbase + 7

    def undo(self):
        super(DeleteCommand, self).undo()

        for imageName in self.imageNames:
            image = elements.ImageEntry(self.visual.imagesetEntry)
            self.visual.imagesetEntry.imageEntries.append(image)

            image.name = imageName
            image.setPos(self.oldPositions[imageName])
            image.setRect(self.oldRects[imageName])
            image.offset.setPos(self.oldOffsets[imageName])

        self.visual.dockWidget.refresh()

    def redo(self):
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            self.visual.imagesetEntry.imageEntries.remove(image)

            image.listItem.imageEntry = None
            image.listItem = None

            image.setParentItem(None)
            self.visual.scene().removeItem(image)

            del image

        self.visual.dockWidget.refresh()

        super(DeleteCommand, self).redo()

class ImagesetRenameCommand(commands.UndoCommand):
    """Changes name of the imageset
    """

    def __init__(self, visual, oldName, newName):
        super(ImagesetRenameCommand, self).__init__()

        self.visual = visual

        self.oldName = oldName
        self.newName = newName

        self.refreshText()

    def refreshText(self):
        self.setText("Rename imageset from '%s' to '%s'" % (self.oldName, self.newName))

    def id(self):
        return idbase + 8

    def mergeWith(self, cmd):
        if self.newName == cmd.oldName:
            self.newName = cmd.newName
            self.refreshText()

            return True

        return False

    def undo(self):
        super(ImagesetRenameCommand, self).undo()

        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.name = self.oldName
        self.visual.dockWidget.name.setText(self.oldName)

    def redo(self):
        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.name = self.newName
        if self.visual.dockWidget.name.text() != self.newName:
            self.visual.dockWidget.name.setText(self.newName)

        super(ImagesetRenameCommand, self).redo()

class ImagesetChangeImageCommand(commands.UndoCommand):
    """Changes the underlying image of the imageset
    """

    def __init__(self, visual, oldImageFile, newImageFile):
        super(ImagesetChangeImageCommand, self).__init__()

        self.visual = visual

        self.oldImageFile = oldImageFile
        self.newImageFile = newImageFile

        self.refreshText()

    def refreshText(self):
        self.setText("Change underlying image from '%s' to '%s'" % (self.oldImageFile, self.newImageFile))

    def id(self):
        return idbase + 9

    def mergeWith(self, cmd):
        if self.newImageFile == cmd.oldImageFile:
            self.newImageFile = cmd.newImageFile
            self.refreshText()

            return True

        return False

    def undo(self):
        super(ImagesetChangeImageCommand, self).undo()

        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.loadImage(self.oldImageFile)
        self.visual.dockWidget.image.setText(imagesetEntry.getAbsoluteImageFile())

    def redo(self):
        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.loadImage(self.newImageFile)
        self.visual.dockWidget.image.setText(imagesetEntry.getAbsoluteImageFile())

        super(ImagesetChangeImageCommand, self).redo()

class ImagesetChangeNativeResolutionCommand(commands.UndoCommand):
    """Changes native resolution of the imageset
    """

    def __init__(self, visual, oldHorzRes, oldVertRes, newHorzRes, newVertRes):
        super(ImagesetChangeNativeResolutionCommand, self).__init__()

        self.visual = visual

        self.oldHorzRes = oldHorzRes
        self.oldVertRes = oldVertRes
        self.newHorzRes = newHorzRes
        self.newVertRes = newVertRes

        self.refreshText()

    def refreshText(self):
        self.setText("Change imageset's native resolution to %ix%i" % (self.newHorzRes, self.newVertRes))

    def id(self):
        return idbase + 10

    def mergeWith(self, cmd):
        if self.newHorzRes == cmd.oldHorzRes and self.newVertRes == cmd.oldVertRes:
            self.newHorzRes = cmd.newHorzRes
            self.newVertRes = cmd.newVertRes

            self.refreshText()

            return True

        return False

    def undo(self):
        super(ImagesetChangeNativeResolutionCommand, self).undo()

        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.nativeHorzRes = self.oldHorzRes
        imagesetEntry.nativeVertRes = self.oldVertRes
        self.visual.dockWidget.nativeHorzRes.setText(str(self.oldHorzRes))
        self.visual.dockWidget.nativeVertRes.setText(str(self.oldVertRes))

    def redo(self):
        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.nativeHorzRes = self.newHorzRes
        imagesetEntry.nativeVertRes = self.newVertRes
        self.visual.dockWidget.nativeHorzRes.setText(str(self.newHorzRes))
        self.visual.dockWidget.nativeVertRes.setText(str(self.newVertRes))

        super(ImagesetChangeNativeResolutionCommand, self).redo()

class ImagesetChangeAutoScaledCommand(commands.UndoCommand):
    """Changes auto scaled value
    """

    def __init__(self, visual, oldAutoScaled, newAutoScaled):
        super(ImagesetChangeAutoScaledCommand, self).__init__()

        self.visual = visual

        self.oldAutoScaled = oldAutoScaled
        self.newAutoScaled = newAutoScaled

        self.refreshText()

    def refreshText(self):
        self.setText("Imageset auto scaled changed to %s" % (self.newAutoScaled))

    def id(self):
        return idbase + 11

    def mergeWith(self, cmd):
        if self.newAutoScaled == cmd.oldAutoScaled:
            self.newAutoScaled = cmd.newAutoScaled
            self.refreshText()

            return True

        return False

    def undo(self):
        super(ImagesetChangeAutoScaledCommand, self).undo()

        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.autoScaled = self.oldAutoScaled

        index = self.visual.dockWidget.autoScaled.findText(imagesetEntry.autoScaled)
        self.visual.dockWidget.autoScaled.setCurrentIndex(index)

    def redo(self):
        imagesetEntry = self.visual.imagesetEntry
        imagesetEntry.autoScaled = self.newAutoScaled

        index = self.visual.dockWidget.autoScaled.findText(imagesetEntry.autoScaled)
        self.visual.dockWidget.autoScaled.setCurrentIndex(index)

        super(ImagesetChangeAutoScaledCommand, self).redo()

class DuplicateCommand(commands.UndoCommand):
    """Duplicates given image entries
    """

    def __init__(self, visual, newNames, newPositions, newRects, newOffsets):
        super(DuplicateCommand, self).__init__()

        self.visual = visual

        self.newNames = newNames
        self.newPositions = newPositions
        self.newRects = newRects
        self.newOffsets = newOffsets

        if len(self.newNames) == 1:
            self.setText("Duplicate image")
        else:
            self.setText("Duplicate %i images" % (len(self.newNames)))

    def id(self):
        return idbase + 12

    def undo(self):
        super(DuplicateCommand, self).undo()

        for imageName in self.newNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            self.visual.imagesetEntry.imageEntries.remove(image)

            image.listItem.imageEntry = None
            image.listItem = None

            image.setParentItem(None)
            self.visual.scene().removeItem(image)

            del image

        self.visual.dockWidget.refresh()

    def redo(self):
        for imageName in self.newNames:
            image = elements.ImageEntry(self.visual.imagesetEntry)
            self.visual.imagesetEntry.imageEntries.append(image)

            image.name = imageName
            image.setPos(self.newPositions[imageName])
            image.setRect(self.newRects[imageName])
            image.offset.setPos(self.newOffsets[imageName])

        self.visual.dockWidget.refresh()

        super(DuplicateCommand, self).redo()

class PasteCommand(commands.UndoCommand):
    """This command pastes clipboard data to the given imageset.
    Based on DuplicateCommand.
    """

    def __init__(self, visual, newNames, newPositions, newRects, newOffsets):
        super(PasteCommand, self).__init__()

        self.visual = visual

        self.newNames = newNames
        self.newPositions = newPositions
        self.newRects = newRects
        self.newOffsets = newOffsets

        self.refreshText()

    def refreshText(self):
        if len(self.newNames) == 1:
            self.setText("Paste image")
        else:
            self.setText("Paste %i images" % (len(self.newNames)))

    def id(self):
        return idbase + 13

    def undo(self):
        super(PasteCommand, self).undo()

        for imageName in self.newNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            self.visual.imagesetEntry.imageEntries.remove(image)

            image.listItem.imageEntry = None
            image.listItem = None

            image.setParentItem(None)
            self.visual.scene().removeItem(image)

            del image

        self.visual.dockWidget.refresh()

    def redo(self):
        for imageName in self.newNames:
            image = elements.ImageEntry(self.visual.imagesetEntry)
            self.visual.imagesetEntry.imageEntries.append(image)

            image.name = imageName
            image.setPos(self.newPositions[imageName])
            image.setRect(self.newRects[imageName])
            image.offset.setPos(self.newOffsets[imageName])

        self.visual.dockWidget.refresh()

        super(PasteCommand, self).redo()
*/
