#ifndef CEGUIPROJECT_H
#define CEGUIPROJECT_H

#include "qstring.h"

// Incapsulates a single CEGUI (CEED) project info and methods to work with it

class CEGUIProject
{
public:

    CEGUIProject();

    bool loadFromFile(const QString& fileName);
    bool save(const QString& fileName = QString());
    void unload();

    bool isModified() const { return changed; }
    QString getAbsolutePathOf(const QString& relPath) const;

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

    bool changed = true; // A new project is not saved yet
};

#endif // CEGUIPROJECT_H
