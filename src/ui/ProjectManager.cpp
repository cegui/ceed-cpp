#include "src/ui/ProjectManager.h"
#include "src/proj/CEGUIProject.h"
#include "src/proj/CEGUIProjectItem.h"
#include "ui_ProjectManager.h"
#include "qinputdialog.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qmenu.h"

ProjectManager::ProjectManager(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ProjectManager)
{
    ui->setupUi(this);

    ui->view->sortByColumn(0, Qt::AscendingOrder);
    ui->view->setContextMenuPolicy(Qt::CustomContextMenu);

    _contextMenu = new QMenu(this);
    _contextMenu->addAction(ui->actionCreateFolder);
    _contextMenu->addSeparator();
    _contextMenu->addAction(ui->actionNewFile);
    _contextMenu->addAction(ui->actionExistingFiles);
    _contextMenu->addSeparator();
    _contextMenu->addAction(ui->actionRename);
    _contextMenu->addAction(ui->actionRemove);

    setProject(nullptr);
}

ProjectManager::~ProjectManager()
{
    delete ui;
}

void ProjectManager::setProject(CEGUIProject* project)
{
    _project = project;
    setEnabled(!!project);
    ui->view->setModel(project);
}

void ProjectManager::on_view_doubleClicked(const QModelIndex& index)
{
    if (!index.isValid() || !index.model()) return;

    //???can obtain item pointer itself?
    CEGUIProjectItem::Type type = CEGUIProjectItem::getItemType(index);
    if (type == CEGUIProjectItem::Type::File)
    {
        const CEGUIProject* project = static_cast<const CEGUIProject*>(index.model());
        emit itemOpenRequested(project->getAbsolutePathOf(index.data(Qt::UserRole + 2).toString()));
    }
}

void ProjectManager::on_view_customContextMenuRequested(const QPoint& pos)
{
    if (!isEnabled()) return;

    auto selectedIndices = ui->view->selectionModel()->selectedIndexes();
    const bool createElement = (selectedIndices.empty() || CEGUIProjectItem::isFolder(selectedIndices[0]));

    ui->actionCreateFolder->setEnabled(createElement);
    ui->actionNewFile->setEnabled(createElement);
    ui->actionExistingFiles->setEnabled(createElement);
    ui->actionRename->setEnabled(selectedIndices.size() == 1);
    ui->actionRemove->setEnabled(!selectedIndices.empty());

    _contextMenu->exec(mapToGlobal(pos));
}

void ProjectManager::on_actionCreateFolder_triggered()
{
    if (!_project) return;

    // TODO: Name clashes!

    bool ok = false;
    auto text = QInputDialog::getText(this,
                                      "Create a folder (only affects project file)",
                                      "Name",
                                      QLineEdit::Normal,
                                      "New folder",
                                      &ok);

    if (!ok) return;

    auto item = new CEGUIProjectItem(_project);
    item->setType(CEGUIProjectItem::Type::Folder);
    item->setPath(text);

    auto parentIndex = ui->view->selectionModel()->currentIndex();
    if (parentIndex.isValid() && CEGUIProjectItem::isFolder(parentIndex))
    {
        CEGUIProjectItem* parentItem = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(parentIndex));
        assert(parentItem);
        parentItem->appendRow(item);

        /* TODO: remove if not needed. Example of model-agnostic child adding code.

            if (model->columnCount(parentIndex) == 0) {
                if (!model->insertColumn(0, parentIndex))
                    return;
            }

            if (!model->insertRow(0, parentIndex))
                return;

            QModelIndex newIndex = model->index(0, 0, parentIndex);
            model->setData(newIndex, QVariant(), Qt::EditRole);
        */
    }
    else
    {
        _project->appendRow(item);
    }
}

void ProjectManager::on_actionNewFile_triggered()
{
    // TODO: name clashes, duplicates

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Create a new file and add it to the project",
                                                    _project->getAbsolutePathOf(""));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error creating file!",
                              "CEED encountered an error trying to create a new file " + fileName);
        return;
    }
    file.close();

    auto item = new CEGUIProjectItem(_project);
    item->setType(CEGUIProjectItem::Type::File);
    item->setPath(_project->getRelativePathOf(fileName));

    auto parentIndex = ui->view->selectionModel()->currentIndex();
    if (parentIndex.isValid() && CEGUIProjectItem::isFolder(parentIndex))
    {
        CEGUIProjectItem* parentItem = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(parentIndex));
        assert(parentItem);
        parentItem->appendRow(item);
    }
    else
    {
        _project->appendRow(item);
    }
}

void ProjectManager::on_actionExistingFiles_triggered()
{
    // TODO: name clashes, duplicates

/*
        files, _ = QtGui.QFileDialog.getOpenFileNames(self,
                                                           "Select one or more files to add to the project",
                                                           self.project.getAbsolutePathOf(""))
        selectedIndices = self.view.selectedIndexes()

        # lets see if file user wants added isn't already there
        previousResponse = None
        for file_ in files:
            if self.view.model().referencesFilePath(file_):
                # file is already in the project
                response = None

                if previousResponse == QtGui.QMessageBox.YesToAll:
                    response = QtGui.QMessageBox.YesToAll
                elif previousResponse == QtGui.QMessageBox.NoToAll:
                    response = QtGui.QMessageBox.NoToAll
                else:
                    response = QtGui.QMessageBox.question(self, "File is already in the project!",
                            "File '%s' that you are trying to add is already referenced in the "
                            "project.\n\n"
                            "Do you want to add it as a duplicate?" % (file_),
                            QtGui.QMessageBox.Yes | QtGui.QMessageBox.No |
                            QtGui.QMessageBox.YesToAll | QtGui.QMessageBox.NoToAll,
                            QtGui.QMessageBox.Yes)

                    previousResponse = response

                if response in [QtGui.QMessageBox.No, QtGui.QMessageBox.NoToAll]:
                    continue

            item = Item(self.project)
            item.itemType = Item.File
            item.path = self.project.getRelativePathOf(file_)

            if len(selectedIndices) == 0:
                self.project.appendRow(item)

            else:
                assert(len(selectedIndices) == 1)

                parent = self.getItemFromModelIndex(selectedIndices[0])
                assert(parent.itemType == Item.Folder)

                parent.appendRow(item)
*/
}

void ProjectManager::on_actionRename_triggered()
{

}

void ProjectManager::on_actionRemove_triggered()
{

}

/*
    def slot_renameAction(self):
        # TODO: Name clashes!

        selectedIndices = self.view.selectedIndexes()
        assert(len(selectedIndices) == 1)

        item = self.getItemFromModelIndex(selectedIndices[0])
        if item.itemType == Item.File:
            text, ok = QtGui.QInputDialog.getText(self,
                                                  "Rename file (renames the file on the disk!)",
                                                  "New name",
                                                  QtGui.QLineEdit.Normal,
                                                  os.path.basename(item.path))

            if ok and text != os.path.basename(item.path):
                # legit change
                newPath = os.path.join(os.path.dirname(item.path), text)

                try:
                    os.rename(self.project.getAbsolutePathOf(item.path), self.project.getAbsolutePathOf(newPath))
                    item.path = newPath
                    self.project.changed = True

                except OSError:
                    QtGui.QMessageBox.question(self,
                                               "Can't rename!",
                                               "Renaming file '%s' to '%s' failed. Exception details follow:\n%s" % (item.path, newPath, sys.exc_info()[1]),
                                               QtGui.QMessageBox.Ok)

        elif item.itemType == Item.Folder:
            text, ok = QtGui.QInputDialog.getText(self,
                                                  "Rename folder (only affects the project file)",
                                                  "New name",
                                                  QtGui.QLineEdit.Normal,
                                                  item.name)

            if ok and text != item.name:
                item.name = text
                self.project.changed = True

    def slot_removeAction(self):
        if not self.isEnabled():
            return

        selectedIndices = self.view.selectedIndexes()
        # when this is called the selection must not be empty
        assert(len(selectedIndices) > 0)

        removeSpec = ""
        if len(selectedIndices) == 1:
            item = self.getItemFromModelIndex(selectedIndices[0])
            removeSpec = "'%s'" % (item.label)
        else:
            removeSpec = "%i project items" % (len(selectedIndices))

        # we have changes, lets ask the user whether we should dump them or save them
        result = QtGui.QMessageBox.question(self,
                                            "Remove items?",
                                            "Are you sure you want to remove %s from the project? "
                                            "This action can't be undone! "
                                            "(Pressing Cancel will cancel the operation!)" % (removeSpec),
                                            QtGui.QMessageBox.Yes | QtGui.QMessageBox.Cancel,
                                            QtGui.QMessageBox.Cancel)

        if result == QtGui.QMessageBox.Cancel:
            # user chickened out ;-)
            return

        elif result == QtGui.QMessageBox.Yes:
            selectedIndices = sorted(selectedIndices, key = lambda index: index.row(), reverse = True)
            removeCount = 0

            # we have to remove files first because multi-selection could screw us otherwise
            # (Parent also removes it's children)

            # first remove files
            for index in selectedIndices:
                item = self.getItemFromModelIndex(index)

                if not item:
                    continue

                if (item.itemType == Item.File):
                    index.model().removeRow(index.row(), index.parent())
                    removeCount += 1

            # then remove folders
            for index in selectedIndices:
                item = self.getItemFromModelIndex(index)

                if not item:
                    continue

                if (item.itemType == Item.Folder):
                    index.model().removeRow(index.row(), index.parent())
                    removeCount += 1

            if len(selectedIndices) - removeCount > 0:
                logging.error("%i selected project items are unknown and can't be deleted", len(selectedIndices))

            if removeCount > 0:
                self.project.changed = True
*/
