#ifndef CEGUIPROJECT_H
#define CEGUIPROJECT_H

#include "qstring.h"
#include "qstandarditemmodel.h"

// Incapsulates a single CEGUI (CEED) project info and methods to work with it

class CEGUIProjectItem;

class CEGUIProject : public QStandardItemModel
{
public:

    CEGUIProject();
    virtual ~CEGUIProject() override;

    virtual Qt::DropActions supportedDragActions() const override;

    bool loadFromFile(const QString& fileName);
    bool save(const QString& fileName = QString());
    void unload();

    bool checkAllDirectories() const;

    bool isModified() const { return changed; }
    void setModified() { changed = true; }

    QString getAbsolutePathOf(const QString& relPath) const;
    QString getRelativePathOf(const QString& absPath) const;
    QString getResourceFilePath(const QString& fileName, const QString& resourceGroup) const;
    bool referencesFilePath(const QString& filePath) const;

private:

    QString defaultResolution;
    QString CEGUIVersion;
    QString filePath;
    QString baseDirectory;
    QString imagesetsPath;
    QString fontsPath;
    QString looknfeelsPath;
    QString schemesPath;
    QString layoutsPath;
    QString xmlSchemasPath;

    CEGUIProjectItem* itemPrototype = nullptr;

    bool changed = true; // A new project is not saved yet
};

#endif // CEGUIPROJECT_H
