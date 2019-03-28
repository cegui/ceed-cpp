#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/util/Utils.h"
#include "src/Application.h"
#include "qfilesystemwatcher.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qdom.h"
#include "qpen.h"

ImagesetEntry::ImagesetEntry(ImagesetVisualMode& visualMode)
    : QGraphicsPixmapItem() // Top-level item
    , _visualMode(visualMode)
{
    setShapeMode(BoundingRectShape);
    setCursor(Qt::ArrowCursor);

    transparencyBackground = new QGraphicsRectItem(this);
    transparencyBackground->setFlags(ItemStacksBehindParent);
    transparencyBackground->setBrush(Utils::getCheckerboardBrush());
    transparencyBackground->setPen(QPen(QColor(Qt::transparent)));
}

ImagesetEntry::~ImagesetEntry()
{
    delete imageMonitor;
}

void ImagesetEntry::loadFromElement(const QDomElement& xml)
{
    _name = xml.attribute("name", "Unknown");

    loadImage(xml.attribute("imagefile", ""));

    nativeHorzRes = xml.attribute("nativeHorzRes", "800").toInt();
    nativeVertRes = xml.attribute("nativeVertRes", "600").toInt();

    autoScaled = xml.attribute("autoScaled", "false");

    auto xmlImage = xml.firstChildElement("Image");
    while (!xmlImage.isNull())
    {
        ImageEntry* image = new ImageEntry(this);
        image->loadFromElement(xmlImage);
        imageEntries.push_back(image);

        xmlImage = xmlImage.nextSiblingElement("Image");
    }
}

void ImagesetEntry::saveToElement(QDomElement& xml)
{
    xml.setTagName("Imageset");
    xml.setAttribute("version", "2");

    xml.setAttribute("name", _name);
    xml.setAttribute("imagefile", imageFile);

    xml.setAttribute("nativeHorzRes", QString::number(nativeHorzRes));
    xml.setAttribute("nativeVertRes", QString::number(nativeVertRes));
    xml.setAttribute("autoScaled", autoScaled);

    for (auto& image : imageEntries)
    {
        auto xmlImage = xml.ownerDocument().createElement("Image");
        image->saveToElement(xmlImage);
        xml.appendChild(xmlImage);
    }
}

ImageEntry*ImagesetEntry::createImageEntry()
{
    ImageEntry* image = new ImageEntry(this);
    imageEntries.push_back(image);
    return image;
}

ImageEntry* ImagesetEntry::getImageEntry(const QString& name) const
{
    auto it = std::find_if(imageEntries.begin(), imageEntries.end(), [&name](ImageEntry* ent)
    {
        return ent->name() == name;
    });

    assert(it != imageEntries.end());
    return (it != imageEntries.end()) ? (*it) : nullptr;
}

void ImagesetEntry::removeImageEntry(const QString& name)
{
    auto it = std::find_if(imageEntries.begin(), imageEntries.end(), [&name](ImageEntry* ent)
    {
        return ent->name() == name;
    });

    if (it == imageEntries.end()) return;

    assert(false && "TODO: test deletion, image entry destructor must be called!");

    ImageEntry* image = (*it);

    imageEntries.erase(it);

    image->setParentItem(nullptr);
    _visualMode.scene()->removeItem(image);
}

//Returns an absolute (OS specific!) path of the underlying image
QString ImagesetEntry::getAbsoluteImageFile() const
{
    auto imagesetPath = _visualMode.getEditor().getFilePath();
    return QFileInfo(imagesetPath).dir().absoluteFilePath(imageFile);
}

// Converts given absolute underlying image path relative to the directory where the .imageset file resides
QString ImagesetEntry::convertToRelativeImageFile(const QString& absPath) const
{
    QDir imagesetDir = QFileInfo(_visualMode.getEditor().getFilePath()).dir();
    return QDir::cleanPath(imagesetDir.relativeFilePath(absPath));
}

// Monitor the image with a QFilesystemWatcher, ask user to reload if changes to the file were made
void ImagesetEntry::onImageChangedByExternalProgram()
{
    //???really here? or maybe somewhere in MainWindow?
    //can subscribe on global file watcher, MainWindow will process dialog,
    //and if user chooses to reload file, signal will be emitted or method will be called.

    if (displayingReloadAlert) return;

    displayingReloadAlert = true;

    auto ret = QMessageBox::question(qobject_cast<Application*>(qApp)->getMainWindow(),
                                     QString("Underlying image '%1' has been modified externally!").arg(imageFile),
                                     "The file has been modified outside the CEGUI Unified Editor.\n\nReload the file?\n\n"
                                     "If you select Yes, UNDO HISTORY MIGHT BE PARTIALLY BROKEN UNLESS THE NEW SIZE IS THE "
                                     "SAME OR LARGER THAN THE OLD!",
                                     QMessageBox::No | QMessageBox::Yes,
                                     QMessageBox::No); // defaulting to No is safer IMO

    if (ret == QMessageBox::Yes) loadImage(imageFile);

    displayingReloadAlert = false;
}

// Replaces the underlying image (if any is loaded) to the image on given relative path
// Relative path is relative to the directory where the .imageset file resides
// (which is usually your project's imageset resource group path)
void ImagesetEntry::loadImage(const QString& relPath)
{
    // If imageMonitor is null, then no images are being watched or the
    // editor is first being opened up
    // Otherwise, the image is being changed or switched, and the monitor
    // should update itself accordingly
    if (imageMonitor) imageMonitor->removePath(getAbsoluteImageFile());

    imageFile = relPath;
    QString absPath = getAbsoluteImageFile();
    setPixmap(QPixmap(absPath));
    transparencyBackground->setRect(boundingRect());

    // Go over all image entries and set their position to force them to be constrained
    // to the new pixmap's dimensions
    for (auto& imageEntry : imageEntries)
    {
        imageEntry->setPos(imageEntry->pos());
        imageEntry->updateDockWidget();
    }

    _visualMode.refreshSceneRect();

    if (!imageMonitor)
    {
        imageMonitor = new QFileSystemWatcher();
        //connect(imageMonitor, &QFileSystemWatcher::fileChanged, this, &ImagesetEntry::onImageChangedByExternalProgram);
    }
    imageMonitor->addPath(absPath);
}
