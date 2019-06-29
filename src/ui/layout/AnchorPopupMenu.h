#ifndef ANCHORPOPUPMENU_H
#define ANCHORPOPUPMENU_H

#include <QWidget>

namespace Ui {
class AnchorPopupMenu;
}

class LayoutScene;

class AnchorPopupMenu : public QWidget
{
    Q_OBJECT

public:

    explicit AnchorPopupMenu(LayoutScene& scene, QWidget* parent = nullptr);
    ~AnchorPopupMenu();

private slots:

    void on_actionParentLeft_triggered();

    void on_actionSelfLeft_triggered();

private:

    Ui::AnchorPopupMenu *ui;
    LayoutScene& _scene;
};

#endif // ANCHORPOPUPMENU_H
