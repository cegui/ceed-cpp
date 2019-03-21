#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"
#include "qvariant.h"

// This is the base class for a class that takes a file and allows manipulation with it

class QWidget;
class QMenu;
class QUndoStack;
class QFileSystemWatcher;

typedef std::unique_ptr<class EditorBase> EditorBasePtr;

class EditorBase : public QObject
{
    Q_OBJECT

public:

    EditorBase(/*compatibilityManager, */ const QString& filePath, bool createUndoStack = false);
    virtual ~EditorBase();

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
    virtual void undo();
    virtual void redo();
    virtual void revert();
    virtual void zoomIn() {}
    virtual void zoomOut() {}
    virtual void zoomReset() {}
    //virtual void zoomFit() {}

    virtual QWidget* getWidget() = 0;
    QUndoStack* getUndoStack() const { return undoStack; }
    virtual bool hasChanges() const;
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return _filePath; }
    QString getLabelText() const { return _labelText /* + (hasChanges() ? " *" : "")*/; }

signals:

    void labelChanged();
    void contentsChanged(bool isModified);
    void undoAvailable(bool available, const QString& text);
    void redoAvailable(bool available, const QString& text);

protected:

    void onFileChangedByExternalProgram();

    virtual void setupEditorMenu(QMenu* editorMenu);

    void enableFileMonitoring(bool enable);
    void askForFileReload();

    virtual void getRawData(QByteArray& outRawData) {}

    QFileSystemWatcher* fileMonitor = nullptr; //???one central, compare path?
    QUndoStack* undoStack = nullptr;
    QString _filePath;
    QString _labelText;
    bool _initialized = false;
    bool fileChangedByExternalProgram = false;
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
