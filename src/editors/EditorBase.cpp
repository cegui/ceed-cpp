#include "src/editors/EditorBase.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "qdir.h"
#include "qmenu.h"
#include "qmessagebox.h"
#include "qfilesystemwatcher.h"
#include "qundostack.h"

// Constructs the editor.
// compatibilityManager - manager that should be used to transform data between
//                        various data types using compatibility layers
// filePath - absolute file path of the file that should be opened
EditorBase::EditorBase(/*compatibilityManager, */ const QString& filePath, bool createUndoStack)
{
    _filePath = QDir::cleanPath(filePath);
    _labelText = QFileInfo(filePath).fileName(); //.baseName();
/*
        self.compatibilityManager = compatibilityManager
        self.desiredSavingDataType = "" if self.compatibilityManager is None else self.compatibilityManager.EditorNativeType
        self.nativeData = None

        self.displayingReloadAlert = False
*/

    if (createUndoStack)
    {
        auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

        undoStack = new QUndoStack(this);
        undoStack->setUndoLimit(settings->getEntryValue("global/undo/limit").toInt());
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
        connect(undoStack, &QUndoStack::cleanChanged, [this](bool clear)
        {
            // Clean means that the undo stack is at a state where it's in sync with the underlying file
            // we set the undostack as clean usually when saving the file so we will assume that there
            emit contentsChanged(!clear);
        });
    }
}

EditorBase::~EditorBase()
{
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

// The callback method for external file changes.  This method is immediately called when
// this editor is open.  Otherwise, it's called when the user activates the editor.
//???move all this 'changed' detection to MainWindow, use single monitor etc?
//!!!mark file as changed if fe refuse to reload!
void EditorBase::onFileChangedByExternalProgram()
{
    fileChangedByExternalProgram = true;
    emit fileChangedExternally();
}

// This method loads everything up so this editor is ready to be switched to
void EditorBase::initialize()
{
    assert(!_initialized);
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
    fileChangedByExternalProgram = false;

    _initialized = true;
}

// Cleans up after itself, this is usually called when you want the editor to close
void EditorBase::finalize()
{
    assert(_initialized);
    _initialized = false;
}

// The editor gets "on stage", it's been clicked on and is now the only active one.
// There can be either 0 editors active (blank screen) or exactly 1 active.
void EditorBase::activate(QMenu* editorMenu)
{
    if (editorMenu)
    {
        editorMenu->clear();
        setupEditorMenu(editorMenu);
    }
}

void EditorBase::setupEditorMenu(QMenu* editorMenu)
{
    editorMenu->menuAction()->setVisible(false);
    editorMenu->menuAction()->setEnabled(false);
}

// The editor gets "off stage", user switched to another editor or
// closed the editor (deactivate() and then finalize() is called).
void EditorBase::deactivate()
{
}

// Reinitialises this editor, effectivelly reloading the file off the hard drive again
void EditorBase::reloadData()
{
    //!!!'current' stuff - to MainWindow!
/*
        wasCurrent = self.mainWindow.activeEditor is self
*/
    finalize();
    initialize(); // fileChangedByExternalProgram is set to false inside
/*
        if wasCurrent:
            self.makeCurrent()
*/
}

// Irrevocably destroys data associated with itself
void EditorBase::destroy()
{
}

bool EditorBase::hasChanges() const
{
    return undoStack ? undoStack->isClean() : false;
}

// Causes the editor to save all it's progress to the file.
// targetPath should be absolute file path.
bool EditorBase::saveAs(const QString& targetPath)
{
    // Stop monitoring the file, the changes that are about to occur are not
    // picked up as being from an external program!
    enableFileMonitoring(false);

    QFile file(targetPath);
    if (file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                              "Error saving file!",
                              "CEED encountered an error trying to save the file " + targetPath);
        enableFileMonitoring(true);
        return false;
    }

    QByteArray rawData;
    getRawData(rawData);
/*
    if self.compatibilityManager is not None:
        outputData = self.compatibilityManager.transform(self.compatibilityManager.EditorNativeType, self.desiredSavingDataType, self.nativeData)
*/
    file.write(rawData);
    file.close();

    constexpr bool updateCurrentPath = true;
    if (updateCurrentPath)
    {
        _filePath = targetPath;
        _labelText = QFileInfo(_filePath).fileName(); //.baseName();
        emit labelChanged();
    }

    enableFileMonitoring(true);

    if (undoStack) undoStack->setClean();

    return true;
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
