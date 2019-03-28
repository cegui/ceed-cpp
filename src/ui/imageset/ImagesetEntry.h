#ifndef IMAGESETENTRY_H
#define IMAGESETENTRY_H

#include "qgraphicsitem.h"

// This is the whole imageset containing all the images (ImageEntries).
// The main reason for this is not to have multiple imagesets editing at once but rather
// to have the transparency background working properly.

class QDomElement;
class ImageEntry;
class QFileSystemWatcher;
class ImagesetVisualMode;

class ImagesetEntry : public QGraphicsPixmapItem
{
public:

    ImagesetEntry(ImagesetVisualMode& visualMode);
    ~ImagesetEntry() override;

    void loadFromElement(const QDomElement& xml);
    void saveToElement(QDomElement& xml);
    void loadImage(const QString& relPath);

    QString name() const { return _name; }
    void setName(const QString& newName) { _name = newName; }
    QString getAutoScaled() const { return autoScaled; }
    void setAutoScaled(const QString& newAutoScaled) { autoScaled = newAutoScaled; }
    int getNativeHorzRes() const { return nativeHorzRes; }
    int getNativeVertRes() const { return nativeVertRes; }
    void setNativeRes(int horz, int vert) { nativeHorzRes = horz; nativeVertRes = vert; }

    ImageEntry* createImageEntry();
    ImageEntry* getImageEntry(const QString& name) const;
    void removeImageEntry(const QString& name);
    const std::vector<ImageEntry*>& getImageEntries() const { return imageEntries; }

    bool showOffsets() const { return _showOffsets; }
    void setShowOffsets(bool value) { _showOffsets = value; }

    const QString& getImageFile() const { return imageFile; }
    QString getAbsoluteImageFile() const;
    QString convertToRelativeImageFile(const QString& absPath) const;

protected:// slots:

    void onImageChangedByExternalProgram();

protected:

    ImagesetVisualMode& _visualMode;

    QString _name = "Unknown";
    QString imageFile;
    QString autoScaled = "false";
    int nativeHorzRes = 800;
    int nativeVertRes = 600;
    bool _showOffsets = false;

    std::vector<ImageEntry*> imageEntries;

    QGraphicsRectItem* transparencyBackground = nullptr;

    //???here or in MainWindow?
    QFileSystemWatcher* imageMonitor = nullptr;
    bool displayingReloadAlert = false;
};

#endif // IMAGESETENTRY_H
