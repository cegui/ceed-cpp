#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDockWidget>

// TODO: rename to ProjectWindow / ProjectBrowser / SolutionBrowser / SolutionExplorer!

namespace Ui {
class ProjectManager;
}

class ProjectManager : public QDockWidget
{
    Q_OBJECT

public:
    explicit ProjectManager(QWidget *parent = nullptr);
    ~ProjectManager();

signals:

    void itemOpenRequested(const QString& name);

private:
    Ui::ProjectManager *ui;
};

#endif // PROJECTMANAGER_H
