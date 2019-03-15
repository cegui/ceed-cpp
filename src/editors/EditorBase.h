#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"

// This is the base class for a class that takes a file and allows manipulation
// with it. It occupies exactly 1 tab space.

class QWidget;

typedef std::unique_ptr<class EditorBase> EditorBasePtr;

class EditorBase
{
public:

    EditorBase(/*compatibilityManager, */ const QString& filePath);
    virtual ~EditorBase() {}

    virtual void initialize(/*mainWindow*/);
    virtual void finalize();
    void reloadData();
    void destroy();

    bool save() { return saveAs(_filePath); }
    bool saveAs(const QString& targetPath, bool updateCurrentPath = true);

    virtual QWidget* getWidget() = 0;
    virtual bool hasChanges() const { return false; }
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return _filePath; }
    QString getLabelText() const { return _labelText + (hasChanges() ? " *" : ""); }

protected:

    QString _filePath;
    QString _labelText;
    bool _initialized = false;
};

typedef std::unique_ptr<class EditorFactoryBase> EditorFactoryBasePtr;

class EditorFactoryBase
{
public:

    virtual ~EditorFactoryBase() {}

    virtual QString getFileTypesDescription() const = 0;
    virtual QStringList getFileExtensions() const = 0;
    virtual bool canEditFile(const QString& filePath) const = 0;
    virtual EditorBasePtr create(const QString& filePath) const = 0;
};

#endif // EDITORBASE_H
