#include "src/ui/layout/AnchorPopupMenu.h"
#include "ui_AnchorPopupMenu.h"

AnchorPopupMenu::AnchorPopupMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnchorPopupMenu)
{
    setWindowFlags(windowFlags() | Qt::Popup);

    ui->setupUi(this);
}

AnchorPopupMenu::~AnchorPopupMenu()
{
    delete ui;
}
