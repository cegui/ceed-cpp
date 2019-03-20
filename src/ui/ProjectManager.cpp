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

    auto files = QFileDialog::getOpenFileNames(this, "Select one or more files to add to the project",
                                               _project->getAbsolutePathOf(""));

    auto parentIndex = ui->view->selectionModel()->currentIndex();
    CEGUIProjectItem* parentItem = nullptr;
    if ((parentIndex.isValid() && CEGUIProjectItem::isFolder(parentIndex)))
    {
        parentItem = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(parentIndex));
        assert(parentItem);
    }

    // Lets see if file user wants added isn't already there
    QMessageBox::StandardButton previousResponse = QMessageBox::StandardButton::Default;
    for (const auto& file : files)
    {
        if (_project->referencesFilePath(file))
        {
            // File is already in the project

            QMessageBox::StandardButton response;
            if (previousResponse == QMessageBox::YesToAll)
                response = QMessageBox::YesToAll;
            else if (previousResponse == QMessageBox::NoToAll)
                continue;
            else
            {
                response = QMessageBox::question(this, "File is already in the project!",
                        QString("File '%1' that you are trying to add is already referenced in the "
                        "project.\n\nDo you want to add it as a duplicate?").arg(file),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll,
                        QMessageBox::Yes);

                previousResponse = response;
            }

            if (response == QMessageBox::No || response == QMessageBox::NoToAll)
                continue;
        }

        auto item = new CEGUIProjectItem(_project);
        item->setType(CEGUIProjectItem::Type::File);
        item->setPath(_project->getRelativePathOf(file));

        if (parentItem)
            parentItem->appendRow(item);
        else
            _project->appendRow(item);
    }
}

void ProjectManager::on_actionRename_triggered()
{
    // TODO: Name clashes!

    auto index = ui->view->selectionModel()->currentIndex();
    if (!index.isValid()) return;

    CEGUIProjectItem* item = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(index));
    assert(item);
    switch (item->getType())
    {
        case CEGUIProjectItem::Type::File:
        {
            QString filePath = item->getPath();
            QFileInfo fileInfo(filePath);
            const QString baseName = fileInfo.baseName();
            bool ok = false;
            auto text = QInputDialog::getText(this,
                                              "Rename file (renames the file on the disk!)",
                                              "New name",
                                              QLineEdit::Normal,
                                              baseName,
                                              &ok);

            if (!ok || text == baseName) return;

            // Legit change
            const QString newPath = fileInfo.dir().filePath(text);
            if (!QFile::rename(_project->getAbsolutePathOf(filePath), _project->getAbsolutePathOf(newPath)))
            {
                QMessageBox::question(this, "Can't rename!",
                                      tr("Renaming file '%1' to '%2' failed").arg(filePath, newPath),
                                      QMessageBox::Ok);
                return;
            }

            item->setPath(newPath);

            break;
        }
        case CEGUIProjectItem::Type::Folder:
        {
            QString name = item->getPath();
            bool ok = false;
            auto text = QInputDialog::getText(this,
                                              "Rename folder (only affects the project file)",
                                              "New name",
                                              QLineEdit::Normal,
                                              name,
                                              &ok);

            if (!ok || text == name) return;

            item->setPath(text);

            break;
        }
    }
}

void ProjectManager::on_actionRemove_triggered()
{
    if (!isEnabled()) return;

    auto selectedIndices = ui->view->selectionModel()->selectedIndexes();
    if (selectedIndices.empty()) return;

    QString removeSpec;
    if (selectedIndices.size() == 1)
    {
        CEGUIProjectItem* item = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(selectedIndices[0]));
        removeSpec = "'" + item->text() + "'";
    }
    else
    {
        removeSpec = QString("%1 project items").arg(selectedIndices.size());
    }

    auto result = QMessageBox::question(this,
                                        "Remove items?",
                                        tr("Are you sure you want to remove %1 from the project? "
                                        "This action can't be undone! "
                                        "(Pressing Cancel will cancel the operation!)").arg(removeSpec),
                                        QMessageBox::Yes | QMessageBox::Cancel,
                                        QMessageBox::Cancel);

    if (result != QMessageBox::Yes) return;

    ui->view->setUpdatesEnabled(false);

    // Sort by row descending
    qSort(selectedIndices.begin(), selectedIndices.end(), [](const QModelIndex& a, const QModelIndex& b)
    {
        return a.row() > b.row();
    });

    int removeCount = 0;

    // We have to remove files first because multi-selection could screw us otherwise
    // (Parent also removes it's children)

    // First remove files
    for (auto& index : selectedIndices)
    {
        CEGUIProjectItem* item = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(index));
        if (item && item->getType() == CEGUIProjectItem::Type::File)
        {
            _project->removeRow(index.row(), index.parent());
            ++removeCount;
        }
    }

    // Then remove folders
    for (auto& index : selectedIndices)
    {
        CEGUIProjectItem* item = static_cast<CEGUIProjectItem*>(_project->itemFromIndex(index));
        if (item && item->getType() == CEGUIProjectItem::Type::Folder)
        {
            _project->removeRow(index.row(), index.parent());
            ++removeCount;
        }
    }

    if (selectedIndices.size() > removeCount)
    {
        /*
        logging.error("%i selected project items are unknown and can't be deleted", len(selectedIndices))
        */
    }

    if (removeCount) _project->setModified();

    ui->view->setUpdatesEnabled(true);
}
