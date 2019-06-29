#include "src/ui/layout/AnchorPopupMenu.h"
#include "ui_AnchorPopupMenu.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include <CEGUI/CoordConverter.h>
#include <CEGUI/Window.h>

AnchorPopupMenu::AnchorPopupMenu(LayoutScene& scene, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AnchorPopupMenu)
    , _scene(scene)
{
    setWindowFlags(windowFlags() | Qt::Popup);

    ui->setupUi(this);

    ui->btnParentLeft->setDefaultAction(ui->actionParentLeft);
    ui->btnSelfLeft->setDefaultAction(ui->actionSelfLeft);
}

AnchorPopupMenu::~AnchorPopupMenu()
{
    delete ui;
}

void AnchorPopupMenu::on_actionParentLeft_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(0.f, 0.f, minY, maxY, true);
    close();
}

void AnchorPopupMenu::on_actionSelfLeft_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        assert(target);
        const auto baseSize = target->getBaseSize();
        const float anchorValue = CEGUI::CoordConverter::asRelative(target->getWidget()->getPosition().d_x, baseSize.d_width);
        _scene.setAnchorValues(anchorValue, anchorValue, minY, maxY, true);
    }
    close();
}
