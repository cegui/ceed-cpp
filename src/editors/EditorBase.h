#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"
#include "qvariant.h"

// This is the base class for a class that takes a file and allows manipulation
// with it. It occupies exactly 1 tab space.

//!!!TODO: signals dataChanged, commandsUpdateRequested!

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

    // Application commands implementation
    virtual bool copy() { return false; }
    virtual bool cut() { return false; }
    virtual bool paste() { return false; }
    virtual bool deleteSelected() { return false; }
    virtual void undo() {}
    virtual void redo() {}
    virtual void revert();
    virtual void zoomIn() {}
    virtual void zoomOut() {}
    virtual void zoomReset() {}
    //virtual void zoomFit() {}

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

// For MultiplePossibleFactoriesDialog only
Q_DECLARE_METATYPE(EditorFactoryBase*);

#endif // EDITORBASE_H
