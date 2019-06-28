#ifndef ANCHORPOPUPMENU_H
#define ANCHORPOPUPMENU_H

#include <QWidget>

namespace Ui {
class AnchorPopupMenu;
}

class AnchorPopupMenu : public QWidget
{
    Q_OBJECT

public:
    explicit AnchorPopupMenu(QWidget *parent = nullptr);
    ~AnchorPopupMenu();

private:
    Ui::AnchorPopupMenu *ui;
};

#endif // ANCHORPOPUPMENU_H
