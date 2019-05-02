#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImageOffsetMark.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"

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
        assert(image);
        image->setPos(rec.oldPos);
        image->updateDockWidget();
    }
}

void ImagesetMoveCommand::redo()
{
    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        assert(image);
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
        assert(image);
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
        assert(image);
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
        assert(image);
        image->getOffsetMark()->setPos(rec.oldPos);
    }
}

void ImagesetOffsetMoveCommand::redo()
{
    for (const auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->getImageEntry(rec.name);
        assert(image);
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

ImageRenameCommand::ImageRenameCommand(ImagesetVisualMode& visualMode, const QString& oldName, const QString& newName)
    : _visualMode(visualMode)
    , _oldName(oldName)
    , _newName(newName)
{
    setText(QString("Rename '%1' to '%2'").arg(_oldName, _newName));
}

void ImageRenameCommand::undo()
{
    QUndoCommand::undo();
    auto image = _visualMode.getImagesetEntry()->getImageEntry(_newName);
    assert(image);
    image->setName(_oldName);
    image->updateListItem();
}

void ImageRenameCommand::redo()
{
    auto image = _visualMode.getImagesetEntry()->getImageEntry(_oldName);
    assert(image);
    image->setName(_newName);
    image->updateListItem();
    QUndoCommand::redo();
}

bool ImageRenameCommand::mergeWith(const QUndoCommand* other)
{
    const ImageRenameCommand* otherCmd = dynamic_cast<const ImageRenameCommand*>(other);
    if (!otherCmd || _newName != otherCmd->_oldName) return false;

    // If our old newName is the same as oldName of the command that comes after this command, we can merge them
    _newName = otherCmd->_newName;
    setText(QString("Rename '%1' to '%2'").arg(_oldName, _newName));
    return true;
}

//---------------------------------------------------------------------

ImagePropertyEditCommand::ImagePropertyEditCommand(ImagesetVisualMode& visualMode, const QString& imageName, const QString& propertyName,
                                                   const QVariant& oldValue, const QVariant& newValue)
    : _visualMode(visualMode)
    , _imageName(imageName)
    , _propertyName(propertyName)
    , _oldValue(oldValue)
    , _newValue(newValue)
{
    setText(QString("Change %1 of '%2' to '%3'").arg(_propertyName, _imageName, _newValue.toString()));
}

void ImagePropertyEditCommand::undo()
{
    QUndoCommand::undo();
    auto image = _visualMode.getImagesetEntry()->getImageEntry(_imageName);
    assert(image);
    image->setProperty(_propertyName, _oldValue);
    image->updateDockWidget();
}

void ImagePropertyEditCommand::redo()
{
    auto image = _visualMode.getImagesetEntry()->getImageEntry(_imageName);
    assert(image);
    image->setProperty(_propertyName, _newValue);
    image->updateDockWidget();
    QUndoCommand::redo();
}

bool ImagePropertyEditCommand::mergeWith(const QUndoCommand* other)
{
    const ImagePropertyEditCommand* otherCmd = dynamic_cast<const ImagePropertyEditCommand*>(other);
    if (!otherCmd) return false;

    if (_imageName == otherCmd->_imageName && _propertyName == otherCmd->_propertyName)
    {
        _newValue = otherCmd->_newValue;
        setText(QString("Change %1 of '%2' to '%3'").arg(_propertyName, _imageName, _newValue.toString()));
        return true;
    }

    return false;
}

//---------------------------------------------------------------------

ImagesetCreateCommand::ImagesetCreateCommand(ImagesetVisualMode& visualMode, const QString& name, QPointF pos, QSizeF size, QPoint offset)
    : _visualMode(visualMode)
    , _name(name)
    , _pos(pos)
    , _size(size)
    , _offset(offset)
{
    setText(QString("Create '%1'").arg(_name));
}

void ImagesetCreateCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getImagesetEntry()->removeImageEntry(_name);
    _visualMode.getDockWidget()->refresh();
}

void ImagesetCreateCommand::redo()
{
    auto image = _visualMode.getImagesetEntry()->createImageEntry();
    image->setName(_name);
    image->setPos(_pos);
    image->setRect(0.0, 0.0, _size.width(), _size.height());
    image->setOffsetX(_offset.x());
    image->setOffsetY(_offset.y());
    _visualMode.getDockWidget()->refresh();
    QUndoCommand::redo();
}

//---------------------------------------------------------------------

ImagesetDeleteCommand::ImagesetDeleteCommand(ImagesetVisualMode& visualMode, std::vector<ImagesetDeleteCommand::Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    if (_imageRecords.size() == 1)
        setText(QString("Delete '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Delete %1 images'").arg(_imageRecords.size()));
}

void ImagesetDeleteCommand::undo()
{
    QUndoCommand::undo();

    for (auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->createImageEntry();
        image->setName(rec.name);
        image->setPos(rec.pos);
        image->setRect(0.0, 0.0, rec.size.width(), rec.size.height());
        image->setOffsetX(rec.offset.x());
        image->setOffsetY(rec.offset.y());
    }

    _visualMode.getDockWidget()->refresh();
}

void ImagesetDeleteCommand::redo()
{
    for (auto& rec : _imageRecords)
        _visualMode.getImagesetEntry()->removeImageEntry(rec.name);

    _visualMode.getDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

ImagesetRenameCommand::ImagesetRenameCommand(ImagesetVisualMode& visualMode, const QString& oldName, const QString& newName)
    : _visualMode(visualMode)
    , _oldName(oldName)
    , _newName(newName)
{
    setText(QString("Rename imageset from '%1' to '%2'").arg(_oldName, _newName));
}

void ImagesetRenameCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getImagesetEntry()->setName(_oldName);
    _visualMode.getDockWidget()->refreshImagesetInfo();
}

void ImagesetRenameCommand::redo()
{
    _visualMode.getImagesetEntry()->setName(_newName);
    _visualMode.getDockWidget()->refreshImagesetInfo();
    QUndoCommand::redo();
}

bool ImagesetRenameCommand::mergeWith(const QUndoCommand* other)
{
    const ImagesetRenameCommand* otherCmd = dynamic_cast<const ImagesetRenameCommand*>(other);
    if (!otherCmd || _newName != otherCmd->_oldName) return false;

    _newName = otherCmd->_newName;
    setText(QString("Rename imageset from '%1' to '%2'").arg(_oldName, _newName));
    return true;
}

//---------------------------------------------------------------------

ImagesetChangeImageCommand::ImagesetChangeImageCommand(ImagesetVisualMode& visualMode, const QString& oldName, const QString& newName)
    : _visualMode(visualMode)
    , _oldName(oldName)
    , _newName(newName)
{
    setText(QString("Change underlying image from '%1' to '%2'").arg(_oldName, _newName));
}

void ImagesetChangeImageCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getImagesetEntry()->loadImage(_oldName);
    _visualMode.getDockWidget()->refreshImagesetInfo();
}

void ImagesetChangeImageCommand::redo()
{
    _visualMode.getImagesetEntry()->loadImage(_newName);
    _visualMode.getDockWidget()->refreshImagesetInfo();
    QUndoCommand::redo();
}

bool ImagesetChangeImageCommand::mergeWith(const QUndoCommand* other)
{
    const ImagesetChangeImageCommand* otherCmd = dynamic_cast<const ImagesetChangeImageCommand*>(other);
    if (!otherCmd || _newName != otherCmd->_oldName) return false;

    _newName = otherCmd->_newName;
    setText(QString("Change underlying image from '%1' to '%2'").arg(_oldName, _newName));
    return true;
}

//---------------------------------------------------------------------

ImagesetChangeNativeResolutionCommand::ImagesetChangeNativeResolutionCommand(ImagesetVisualMode& visualMode, QPoint oldResolution, QPoint newResolution)
    : _visualMode(visualMode)
    , _oldResolution(oldResolution)
    , _newResolution(newResolution)
{
    setText(QString("Change imageset's native resolution to %1x%2").arg(_newResolution.x()).arg(_newResolution.y()));
}

void ImagesetChangeNativeResolutionCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getImagesetEntry()->setNativeRes(_oldResolution.x(), _oldResolution.y());
    _visualMode.getDockWidget()->refreshImagesetInfo();
}

void ImagesetChangeNativeResolutionCommand::redo()
{
    _visualMode.getImagesetEntry()->setNativeRes(_newResolution.x(), _newResolution.y());
    _visualMode.getDockWidget()->refreshImagesetInfo();
    QUndoCommand::redo();
}

bool ImagesetChangeNativeResolutionCommand::mergeWith(const QUndoCommand* other)
{
    auto otherCmd = dynamic_cast<const ImagesetChangeNativeResolutionCommand*>(other);
    if (!otherCmd || _newResolution != otherCmd->_oldResolution) return false;

    _newResolution = otherCmd->_newResolution;
    setText(QString("Change imageset's native resolution to %1x%2").arg(_newResolution.x()).arg(_newResolution.y()));
    return true;
}

//---------------------------------------------------------------------

ImagesetChangeAutoScaledCommand::ImagesetChangeAutoScaledCommand(ImagesetVisualMode& visualMode, const QString& oldAutoScaled, const QString& newAutoScaled)
    : _visualMode(visualMode)
    , _oldAutoScaled(oldAutoScaled)
    , _newAutoScaled(newAutoScaled)
{
    setText(QString("Imageset auto scaled changed to %s").arg(_newAutoScaled));
}

void ImagesetChangeAutoScaledCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getImagesetEntry()->setAutoScaled(_oldAutoScaled);
    _visualMode.getDockWidget()->refreshImagesetInfo();
}

void ImagesetChangeAutoScaledCommand::redo()
{
    _visualMode.getImagesetEntry()->setAutoScaled(_newAutoScaled);
    _visualMode.getDockWidget()->refreshImagesetInfo();
    QUndoCommand::redo();
}

bool ImagesetChangeAutoScaledCommand::mergeWith(const QUndoCommand* other)
{
    auto otherCmd = dynamic_cast<const ImagesetChangeAutoScaledCommand*>(other);
    if (!otherCmd || _newAutoScaled != otherCmd->_oldAutoScaled) return false;

    _newAutoScaled = otherCmd->_newAutoScaled;
    setText(QString("Imageset auto scaled changed to %s").arg(_newAutoScaled));
    return true;
}

//---------------------------------------------------------------------

ImagesetDuplicateCommand::ImagesetDuplicateCommand(ImagesetVisualMode& visualMode, std::vector<ImagesetDuplicateCommand::Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    if (_imageRecords.size() == 1)
        setText(QString("Duplicate image '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Duplicate %1 images'").arg(_imageRecords.size()));
}

void ImagesetDuplicateCommand::undo()
{
    QUndoCommand::undo();

    for (auto& rec : _imageRecords)
        _visualMode.getImagesetEntry()->removeImageEntry(rec.name);

    _visualMode.getDockWidget()->refresh();
}

void ImagesetDuplicateCommand::redo()
{
    for (auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->createImageEntry();
        image->setName(rec.name);
        image->setPos(rec.pos);
        image->setRect(0.0, 0.0, rec.size.width(), rec.size.height());
        image->setOffsetX(rec.offset.x());
        image->setOffsetY(rec.offset.y());
    }

    _visualMode.getDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

ImagesetPasteCommand::ImagesetPasteCommand(ImagesetVisualMode& visualMode, std::vector<ImagesetPasteCommand::Record>&& imageRecords)
    : _visualMode(visualMode)
    , _imageRecords(std::move(imageRecords))
{
    if (_imageRecords.size() == 1)
        setText(QString("Paste image '%1'").arg(_imageRecords[0].name));
    else
        setText(QString("Paste %1 images'").arg(_imageRecords.size()));
}

void ImagesetPasteCommand::undo()
{
    QUndoCommand::undo();

    for (auto& rec : _imageRecords)
        _visualMode.getImagesetEntry()->removeImageEntry(rec.name);

    _visualMode.getDockWidget()->refresh();
}

void ImagesetPasteCommand::redo()
{
    for (auto& rec : _imageRecords)
    {
        auto image = _visualMode.getImagesetEntry()->createImageEntry();
        image->setName(rec.name);
        image->setPos(rec.pos);
        image->setRect(0.0, 0.0, rec.size.width(), rec.size.height());
        image->setOffsetX(rec.offset.x());
        image->setOffsetY(rec.offset.y());
    }

    _visualMode.getDockWidget()->refresh();

    QUndoCommand::redo();
}
