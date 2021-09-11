#include "src/editors/EditorBase.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include "qdir.h"
#include "qmenu.h"
#include "qmessagebox.h"
#include "qfilesystemwatcher.h"
#include "qundostack.h"
#include <qfiledialog.h>

// Constructs the editor.
// compatibilityManager - manager that should be used to transform data between
//                        various data types using compatibility layers
// filePath - absolute file path of the file that should be opened
EditorBase::EditorBase(/*compatibilityManager, */ const QString& filePath, bool createUndoStack)
{
    _filePath = QDir::cleanPath(filePath);
/*
        self.compatibilityManager = compatibilityManager
        self.desiredSavingDataType = "" if self.compatibilityManager is None else self.compatibilityManager.EditorNativeType
        self.nativeData = None
*/

    if (createUndoStack)
    {
        auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

        undoStack = new QUndoStack(this);
        undoStack->setUndoLimit(settings->getEntryValue("global/app/undo_limit").toInt());
        undoStack->setClean();

        connect(undoStack, &QUndoStack::canUndoChanged, [this](bool available)
        {
            emit undoAvailable(available, undoStack->undoText());
        });
        connect(undoStack, &QUndoStack::canRedoChanged, [this](bool available)
        {
            emit redoAvailable(available, undoStack->redoText());
        });
        connect(undoStack, &QUndoStack::undoTextChanged, [this](const QString& text)
        {
            emit undoAvailable(undoStack->canUndo(), text);
        });
        connect(undoStack, &QUndoStack::redoTextChanged, [this](const QString& text)
        {
            emit redoAvailable(undoStack->canRedo(), text);
        });
        connect(undoStack, &QUndoStack::indexChanged, [this](int /*idx*/)
        {
            onContentsChanged();
        });
    }
}

EditorBase::~EditorBase()
{
    if (undoStack) undoStack->disconnect();
}

// Adds or removes a file monitor to the specified file so CEED will alert the user
// that an external change happened to the file
void EditorBase::enableFileMonitoring(bool enable)
{
    if (enable)
    {
        // Lazy initialization
        if (!fileMonitor)
        {
            //???use global file monitor?
            fileMonitor = new QFileSystemWatcher(this);
            connect(fileMonitor, &QFileSystemWatcher::fileChanged, this, &EditorBase::onFileChangedByExternalProgram);
        }

        fileMonitor->addPath(_filePath);
    }
    else if (fileMonitor)
    {
        fileMonitor->removePath(_filePath);
    }
}

void EditorBase::markAsUnchanged()
{
    syncStatus = SyncStatus::Sync;
    if (undoStack) undoStack->setClean();

    // Force GUI update (modified mark etc)
    onContentsChanged();
}

// The callback method for external file changes.  This method is immediately called when
// this editor is open. Otherwise, it's called when the user activates the editor.
//???move all this 'changed' detection to MainWindow, use single monitor etc?
void EditorBase::onFileChangedByExternalProgram()
{
    syncStatus = SyncStatus::Conflict;
    _changesDiscarded = false;
    emit fileChangedExternally();
}

void EditorBase::onContentsChanged()
{
    // New changes are not discarded by the user and we should ask about saving them
    _changesDiscarded = false;
    emit contentsChanged(hasChanges());
}

//???TODO: make names better? initialize says nothing about loading from disc etc!
// This method loads everything up so this editor is ready to be switched to
void EditorBase::initialize()
{
    assert(!_initialized);
    if (_initialized) return;

    _labelText = _filePath.isEmpty() ? ("New " + getFileTypesDescription().toLower()) : QFileInfo(_filePath).fileName();

/*
    if self.compatibilityManager is not None:
        rawData = codecs.open(self.filePath, mode = "r", encoding = "utf-8").read()
        rawDataType = ""

        if rawData == "":
            # it's an empty new file, the derived classes deal with this separately
            self.nativeData = rawData

            if mainWindow.project is None:
                self.desiredSavingDataType = self.compatibilityManager.EditorNativeType
            else:
                self.desiredSavingDataType = self.compatibilityManager.getSuitableDataTypeForCEGUIVersion(mainWindow.project.CEGUIVersion)

        else:
            try:
                rawDataType = self.compatibilityManager.guessType(rawData, self.filePath)

                # A file exists and the editor has it open, so watch it for
                # external changes.
                self.addFileMonitor(self.filePath)

            except compatibility.NoPossibleTypesError:
                dialog = NoTypeDetectedDialog(self.compatibilityManager)
                result = dialog.exec_()

                rawDataType = self.compatibilityManager.EditorNativeType
                self.nativeData = ""

                if result == QtGui.QDialog.Accepted:
                    selection = dialog.typeChoice.selectedItems()

                    if len(selection) == 1:
                        rawDataType = selection[0].text()
                        self.nativeData = None

            except compatibility.MultiplePossibleTypesError as e:
                # if no project is opened or if the opened file was detected as something not suitable for the target CEGUI version of the project
                if (mainWindow.project is None) or (self.compatibilityManager.getSuitableDataTypeForCEGUIVersion(mainWindow.project.CEGUIVersion) not in e.possibleTypes):
                    dialog = MultipleTypesDetectedDialog(self.compatibilityManager, e.possibleTypes)
                    result = dialog.exec_()

                    rawDataType = self.compatibilityManager.EditorNativeType
                    self.nativeData = ""

                    if result == QtGui.QDialog.Accepted:
                        selection = dialog.typeChoice.selectedItems()

                        if len(selection) == 1:
                            rawDataType = selection[0].text()
                            self.nativeData = None

                else:
                    rawDataType = self.compatibilityManager.getSuitableDataTypeForCEGUIVersion(mainWindow.project.CEGUIVersion)
                    self.nativeData = None

            # by default, save in the same format as we opened in
            self.desiredSavingDataType = rawDataType

            if mainWindow.project is not None:
                projectCompatibleDataType = self.compatibilityManager.CEGUIVersionTypes[mainWindow.project.CEGUIVersion]

                if projectCompatibleDataType != rawDataType:
                    if QtGui.QMessageBox.question(mainWindow,
                                                  "Convert to format suitable for opened project?",
                                                  "File you are opening isn't suitable for the project that is opened at the moment.\n"
                                                  "Do you want to convert it to a suitable format upon saving?\n"
                                                  "(from '%s' to '%s')\n"
                                                  "Data COULD be lost, make a backup!)" % (rawDataType, projectCompatibleDataType),
                                                  QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, QtGui.QMessageBox.No) == QtGui.QMessageBox.Yes:
                        self.desiredSavingDataType = projectCompatibleDataType

            # if nativeData is "" at this point, data type was not successful and user didn't select
            # any data type as well so we will just use given file as an empty file

            if self.nativeData != "":
                try:
                    self.nativeData = self.compatibilityManager.transform(rawDataType, self.compatibilityManager.EditorNativeType, rawData)

                except compatibility.LayerNotFoundError:
                    # TODO: Dialog, can't convert
                    self.nativeData = ""
*/

    if (undoStack) undoStack->clear();
    _initialized = true;
    enableFileMonitoring(true);
    markAsUnchanged();
}

// Cleans up after itself, this is usually called when you want the editor to close
void EditorBase::finalize()
{
    assert(_initialized);
    _initialized = false;
}

// The editor gets "on stage", it's been clicked on and is now the only active one.
// There can be either 0 editors active (blank screen) or exactly 1 active.
void EditorBase::activate(MainWindow& /*mainWindow*/)
{
}

// The editor gets "off stage", user switched to another editor or
// closed the editor (deactivate() and then finalize() is called).
void EditorBase::deactivate(MainWindow& /*mainWindow*/)
{
}

// Reinitialises this editor, effectivelly reloading the file off the hard drive again
void EditorBase::reloadData()
{
    finalize();
    initialize(); // sync status is updated inside
}

// Irrevocably destroys data associated with itself
void EditorBase::destroy()
{
}

// We have changes if our undo stack is not clear or if our data
// if out of sync with the data on disk.
bool EditorBase::hasChanges() const
{
    if (undoStack && !undoStack->isClean()) return true;
    return syncStatus != SyncStatus::Sync;
}

// Causes the editor to save all it's progress to the file.
// targetPath should be absolute file path.
bool EditorBase::saveAs(const QString& targetPath)
{
    const QString prevFilePath = _filePath;

    QString actualPath;
    if (prevFilePath.isEmpty())
    {
        // Editor target is new, ask for a file location and name

        QString defaultDir;
        if (CEGUIManager::Instance().isProjectLoaded())
            defaultDir = CEGUIManager::Instance().getCurrentProject()->getAbsolutePathOf("");

        QStringList ext = getFileExtensions();

        QStringList filters;
        filters.append(QString("%1 files (%2)").arg(getFileTypesDescription(), "*." + ext.join(" *.")));
        filters.append("All files (*)");

        QFileDialog dialog(getWidget(), ("New " + getFileTypesDescription().toLower()), defaultDir, filters.join(";;"));
        dialog.setDefaultSuffix(ext[0]);

        if (dialog.exec())
            actualPath = dialog.selectedFiles()[0];
        else
            return false;
    }
    else actualPath = targetPath;

    // Stop monitoring the file, the changes that are about to occur are not
    // picked up as being from an external program! Also file renaming breaks
    // watching it, so re-enabling monitor is necessary.
    enableFileMonitoring(false);

    // If the file already exists, rename it to avoid data losing due to save error
    const bool tmpUsed = QFileInfo(actualPath).exists();
    if (tmpUsed)
    {
        QFile prevFile(actualPath);
        if (!prevFile.rename(actualPath + ".bak"))
        {
            QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                                  "Error saving file!",
                                  "CEED encountered an error trying to save the file " + actualPath);
            return false;
        }
    }

    QFile file(actualPath);
    if (file.open(QIODevice::WriteOnly))
    {
        // Do it before obtaining raw data since it may contain relative pathes.
        // For example imageset XML contains a relative path to underlying image.
        _filePath = actualPath;

        QByteArray rawData;
        getRawData(rawData);
        /*
            if self.compatibilityManager is not None:
                outputData = self.compatibilityManager.transform(self.compatibilityManager.EditorNativeType, self.desiredSavingDataType, self.nativeData)
        */

        const auto written = file.write(rawData);
        file.close();

        if (written == rawData.size())
        {
            if (tmpUsed) QFile(actualPath + ".bak").remove();
            enableFileMonitoring(true);

            markAsUnchanged();

            if (prevFilePath != _filePath)
            {
                _labelText = QFileInfo(_filePath).fileName();
                emit filePathChanged(prevFilePath, _filePath);
            }

            return true;
        }
    }

    // Something went wrong, show error and restore backup
    QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                          "Error saving file!",
                          "CEED encountered an error trying to save the file " + actualPath);

    if (tmpUsed)
    {
        QFile(actualPath).remove();
        QFile(actualPath + ".bak").rename(actualPath);
    }
    enableFileMonitoring(true);

    return false;
}

// Either reload file from disk or confirm desynchronization
void EditorBase::resolveSyncConflict(bool reload)
{
    if (syncStatus != SyncStatus::Conflict) return;

    if (reload)
    {
        reloadData();
    }
    else
    {
        syncStatus = SyncStatus::NotSync;

        // Force GUI update (modified mark etc)
        onContentsChanged();
    }
}

bool EditorBase::confirmClosing()
{
    // We can close immediately
    if (_changesDiscarded || !hasChanges()) return true;

    // We have changes, let's ask the user whether we should discard or save them
    auto result = QMessageBox::question(qobject_cast<Application*>(qApp)->getMainWindow(),
                                        "Unsaved changes!",
                                        tr("There are unsaved changes in '%1'. "
                                        "Do you want to save them? "
                                        "(Pressing Discard will discard the changes!)").arg(getFilePath()),
                                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                        QMessageBox::Save);

    if (result == QMessageBox::Save)
    {
        // Let's save changes and then kill the editor (This is the default action)
        // If there was an error saving the file, stop what we're doing
        // and let the user fix the problem.
        return save();
    }
    else if (result == QMessageBox::Discard)
    {
        // Changes will be discarded. Flag is set instead of actually discarding changes which is
        // potentially wasteful because the editor is going to be closed soon in most of cases.
        _changesDiscarded = true;
        return true;
    }

    // Do nothing if user selected 'Cancel'
    return false;
}

void EditorBase::undo()
{
    if (undoStack) undoStack->undo();
}

void EditorBase::redo()
{
    if (undoStack) undoStack->redo();
}

// Causes the editor to discard all it's progress
void EditorBase::revert()
{
    if (!hasChanges()) return;

    if (undoStack && undoStack->cleanIndex() >= 0)
    {
        // If we have undo stack, we can simply use it instead of the slower reinitialisation approach
        undoStack->setIndex(undoStack->cleanIndex());
    }
    else
    {
        // It is possible that we can't undo/redo to cleanIndex. This can happen because of undo limitations.
        // Use the default but kind of wasteful implementation.
        reloadData();
    }
}

/*
    def getDesiredSavingDataType(self):
        """Returns current desired saving data type. Data type that will be used when user requests to save this file
        """

        return self.desiredSavingDataType if self.compatibilityManager is not None else None
*/

//---------------------------------------------------------------------

bool EditorFactoryBase::canEditFile(const QString& filePath) const
{
    return getFileExtensions().contains(QFileInfo(filePath).completeSuffix());
}
