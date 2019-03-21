#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"
#include "qvariant.h"

// This is the base class for a class that takes a file and allows manipulation with it

class QWidget;
class QMenu;
class QUndoStack;

typedef std::unique_ptr<class EditorBase> EditorBasePtr;

class EditorBase
{
public:

    EditorBase(/*compatibilityManager, */ const QString& filePath);
    virtual ~EditorBase() {}

    virtual void initialize();
    virtual void finalize();
    virtual void activate(QMenu* editorMenu);
    virtual void deactivate();
    void reloadData();
    void destroy();

    bool save() { return saveAs(_filePath); }
    bool saveAs(const QString& targetPath);

    // Application commands implementation
    virtual void copy() {}
    virtual void cut() {}
    virtual void paste() {}
    virtual void deleteSelected() {}
    virtual void undo() {}
    virtual void redo() {}
    virtual void revert();
    virtual void zoomIn() {}
    virtual void zoomOut() {}
    virtual void zoomReset() {}
    //virtual void zoomFit() {}

    virtual QWidget* getWidget() = 0;
    QUndoStack* getUndoStack() const { return nullptr; } //!!!TODO: implement!
    virtual bool hasChanges() const { return false; }
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return _filePath; }
    QString getLabelText() const { return _labelText + (hasChanges() ? " *" : ""); }

protected:

    virtual void setupEditorMenu(QMenu* editorMenu);
    void enableFileMonitoring(bool enable);
    virtual void getRawData(QByteArray& outRawData) {}

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
    virtual bool canEditFile(const QString& filePath) const;
    virtual EditorBasePtr create(const QString& filePath) const = 0;
};

// For MultiplePossibleFactoriesDialog only
Q_DECLARE_METATYPE(EditorFactoryBase*);

#endif // EDITORBASE_H
