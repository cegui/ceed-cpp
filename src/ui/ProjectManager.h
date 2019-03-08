#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDockWidget>

namespace Ui {
class ProjectManager;
}

class ProjectManager : public QDockWidget
{
    Q_OBJECT

public:
    explicit ProjectManager(QWidget *parent = nullptr);
    ~ProjectManager();

private:
    Ui::ProjectManager *ui;
};

#endif // PROJECTMANAGER_H
