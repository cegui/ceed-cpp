#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDockWidget>

// TODO: rename to ProjectWindow / ProjectBrowser / SolutionBrowser / SolutionExplorer!

namespace Ui {
class ProjectManager;
}

class CEGUIProject;
class QMenu;

class ProjectManager : public QDockWidget
{
    Q_OBJECT

public:

    explicit ProjectManager(QWidget *parent = nullptr);
    ~ProjectManager();

    void setProject(CEGUIProject* project = nullptr);

signals:

    void itemOpenRequested(const QString& name);

private slots:
    void on_view_doubleClicked(const QModelIndex &index);

    void on_view_customContextMenuRequested(const QPoint &pos);

    void on_actionCreateFolder_triggered();

    void on_actionNewFile_triggered();

    void on_actionExistingFiles_triggered();

    void on_actionRename_triggered();

    void on_actionRemove_triggered();

private:

    Ui::ProjectManager *ui;
    QMenu* _contextMenu = nullptr;
    CEGUIProject* _project = nullptr;
};

#endif // PROJECTMANAGER_H
