#ifndef IMAGESETENTRY_H
#define IMAGESETENTRY_H

#include "qgraphicsitem.h"

// This is the whole imageset containing all the images (ImageEntries).
// The main reason for this is not to have multiple imagesets editing at once but rather
// to have the transparency background working properly.

class QDomElement;
class ImageEntry;
class QFileSystemWatcher;

class ImagesetEntry : public QGraphicsPixmapItem
{
public:

    ImagesetEntry(QGraphicsItem *parent = nullptr);
    ~ImagesetEntry() override;

    void loadFromElement(const QDomElement& xml);
    void saveToElement(QDomElement& xml);

    ImageEntry* getImageEntry(const QString& name) const;

    QString getAbsoluteImageFile() const;
    QString convertToRelativeImageFile(const QString& absPath) const;

protected:// slots:

    void onImageChangedByExternalProgram();

protected:

    void loadImage(const QString& relPath);

    QString _name = "Unknown";
    QString imageFile;
    QString autoScaled = "false";
    int nativeHorzRes = 800;
    int nativeVertRes = 600;
    bool showOffsets = false;

    std::vector<ImageEntry*> imageEntries;

    QFileSystemWatcher* imageMonitor = nullptr;
    QGraphicsRectItem* transparencyBackground = nullptr;
};

#endif // IMAGESETENTRY_H
