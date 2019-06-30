#ifndef ANCHORPOPUPMENU_H
#define ANCHORPOPUPMENU_H

#include <QWidget>

// Popup menu with preset buttons for anchor positioning. There are
// most common patterns of anchor setup. You may add your own by code.
// TODO: add tuning - what actions are visible. The full list may become too long.

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
    void on_actionParentTop_triggered();
    void on_btnParentLeftTop_clicked();
    void on_actionSelfLeft_triggered();
    void on_actionSelfTop_triggered();
    void on_btnSelfLeftTop_clicked();

    void on_actionParentHCenter_triggered();

    void on_actionParentVCenter_triggered();

    void on_actionSelfHCenter_triggered();

    void on_actionSelfVCenter_triggered();

    void on_btnParentCenter_clicked();

    void on_btnSelfCenter_clicked();

private:

    Ui::AnchorPopupMenu *ui;
    LayoutScene& _scene;
};

#endif // ANCHORPOPUPMENU_H
