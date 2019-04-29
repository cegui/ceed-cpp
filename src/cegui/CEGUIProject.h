#ifndef CEGUIPROJECT_H
#define CEGUIPROJECT_H

#include "qstring.h"
#include "qstandarditemmodel.h"

// Incapsulates a single CEGUI (CEED) project info and methods to work with it

class CEGUIProject : public QStandardItemModel
{
public:

    //!!!EditorEmbeddedCEGUIVersion must be a version string from the current CEGUI system!
    static const QString EditorEmbeddedCEGUIVersion;
    static const QStringList CEGUIVersions;

    CEGUIProject();
    virtual ~CEGUIProject() override;

    virtual Qt::DropActions supportedDragActions() const override;

    bool loadFromFile(const QString& fileName);
    bool save(const QString& newFilePath = QString());
    void unload();

    bool checkAllDirectories() const;

    bool isModified() const { return changed; }
    void setModified() { changed = true; }

    QString getAbsolutePathOf(const QString& relPath) const;
    QString getRelativePathOf(const QString& absPath) const;
    QString getResourceFilePath(const QString& fileName, const QString& resourceGroup) const;
    bool referencesFilePath(const QString& filePath) const;

    void setDefaultResolution(const QString& string);
    const QSize& getDefaultResolution() const { return defaultResolution; }
    QString getDefaultResolutionString() const;

//private:
public: // For now, to avoid lots of boilerplate setters & getters

    QString CEGUIVersion;
    QString filePath;
    QString baseDirectory;
    QString imagesetsPath;
    QString fontsPath;
    QString looknfeelsPath;
    QString schemesPath;
    QString layoutsPath;
    QString xmlSchemasPath;

private:

    QSize defaultResolution;

    bool changed = true; // A new project is not saved yet
};

#endif // CEGUIPROJECT_H
