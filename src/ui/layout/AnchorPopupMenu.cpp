#include "src/ui/layout/AnchorPopupMenu.h"
#include "ui_AnchorPopupMenu.h"

AnchorPopupMenu::AnchorPopupMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnchorPopupMenu)
{
    setWindowFlags(windowFlags() | Qt::Popup);

    ui->setupUi(this);

    // QToolButton::setDefaultAction
    // https://wiki.qt.io/PushButton_Based_On_Action
}

AnchorPopupMenu::~AnchorPopupMenu()
{
    delete ui;
}
