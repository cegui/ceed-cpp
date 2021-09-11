#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"
#include "qvariant.h"
#include <memory>

// This is the base class for a class that takes a file and allows manipulation with it

class QWidget;
class QUndoStack;
class QFileSystemWatcher;
class MainWindow;

typedef std::unique_ptr<class EditorBase> EditorBasePtr;

class EditorBase : public QObject
{
    Q_OBJECT

public:

    // Synchronization status between our editor and the file on disk.
    enum class SyncStatus
    {
        Sync,       // Our editor is in sync with the file on disk
        NotSync,    // Our editor is out of sync (file was modified externally and we kept our version)
        Conflict    // Our editor is out of sync and waits for the user to resolve state
    };

    EditorBase(/*compatibilityManager, */ const QString& filePath, bool createUndoStack = false);
    virtual ~EditorBase();

    virtual void initialize();
    virtual void finalize();
    virtual void activate(MainWindow& mainWindow);
    virtual void deactivate(MainWindow& mainWindow);
    void reloadData();
    void destroy();

    bool save() { return saveAs(_filePath); }
    bool saveAs(const QString& targetPath);
    void resolveSyncConflict(bool reload);
    bool confirmClosing();

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
    bool isModifiedExternally() const { return syncStatus == SyncStatus::Conflict; }
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return _filePath; }
    QString getLabelText() const { return _labelText; }

    virtual QString getFileTypesDescription() const = 0;
    virtual QStringList getFileExtensions() const = 0;

signals:

    void filePathChanged(const QString& oldPath, const QString& newPath);
    void contentsChanged(bool isModified);
    void fileChangedExternally();
    void undoAvailable(bool available, const QString& text);
    void redoAvailable(bool available, const QString& text);

protected:

    void onFileChangedByExternalProgram();
    void onContentsChanged();

    void enableFileMonitoring(bool enable);

    virtual void getRawData(QByteArray& /*outRawData*/) {}
    virtual void markAsUnchanged();

    QFileSystemWatcher* fileMonitor = nullptr; //???one central, compare path?
    QUndoStack* undoStack = nullptr;
    QString _filePath;
    QString _labelText;
    SyncStatus syncStatus = SyncStatus::Sync;
    bool _initialized = false;
    bool _changesDiscarded = false; // Stores this instead of discarding changes actually
};

typedef std::unique_ptr<class EditorFactoryBase> EditorFactoryBasePtr;

class EditorFactoryBase
{
public:

    virtual ~EditorFactoryBase() {}

    virtual QString getFileTypesDescription() const = 0;
    virtual QStringList getFileExtensions() const = 0;
    virtual bool canEditFile(const QString& filePath) const;
    virtual bool requiresProject() const { return false; }
    virtual EditorBasePtr create(const QString& filePath) const = 0;
};

// For MultiplePossibleFactoriesDialog only
Q_DECLARE_METATYPE(EditorFactoryBase*);

#endif // EDITORBASE_H
