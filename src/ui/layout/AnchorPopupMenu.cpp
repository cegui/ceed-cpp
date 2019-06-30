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
    ui->btnParentTop->setDefaultAction(ui->actionParentTop);
    ui->btnSelfTop->setDefaultAction(ui->actionSelfTop);
    ui->btnParentHCenter->setDefaultAction(ui->actionParentHCenter);
    ui->btnSelfHCenter->setDefaultAction(ui->actionSelfHCenter);
    ui->btnParentVCenter->setDefaultAction(ui->actionParentVCenter);
    ui->btnSelfVCenter->setDefaultAction(ui->actionSelfVCenter);
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

void AnchorPopupMenu::on_actionParentTop_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.f, 0.f, true);
    close();
}

void AnchorPopupMenu::on_btnParentLeftTop_clicked()
{
    _scene.setAnchorValues(0.f, 0.f, 0.f, 0.f, true);
    close();
}

void AnchorPopupMenu::on_actionSelfLeft_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const float anchorValue = CEGUI::CoordConverter::asRelative(target->getWidget()->getPosition().d_x, baseSize.d_width);
        _scene.setAnchorValues(anchorValue, anchorValue, minY, maxY, true);
    }
    close();
}

void AnchorPopupMenu::on_actionSelfTop_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const float anchorValue = CEGUI::CoordConverter::asRelative(target->getWidget()->getPosition().d_y, baseSize.d_height);
        _scene.setAnchorValues(minX, maxX, anchorValue, anchorValue, true);
    }
    close();
}

void AnchorPopupMenu::on_btnSelfLeftTop_clicked()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto anchorValue = CEGUI::CoordConverter::asRelative(target->getWidget()->getPosition(), baseSize);
        _scene.setAnchorValues(anchorValue.x, anchorValue.x, anchorValue.y, anchorValue.y, true);
    }
    close();
}

void AnchorPopupMenu::on_actionParentHCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(0.5f, 0.5f, minY, maxY, true);
    close();
}

void AnchorPopupMenu::on_actionParentVCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.5f, 0.5f, true);
    close();
}

void AnchorPopupMenu::on_btnParentCenter_clicked()
{
    _scene.setAnchorValues(0.5f, 0.5f, 0.5f, 0.5f, true);
    close();
}

void AnchorPopupMenu::on_actionSelfHCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto pt = target->getWidget()->getPosition().d_x + target->getWidget()->getSize().d_width * 0.5f;
        const float anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize.d_width);
        _scene.setAnchorValues(anchorValue, anchorValue, minY, maxY, true);
    }
    close();
}

void AnchorPopupMenu::on_actionSelfVCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto pt = target->getWidget()->getPosition().d_y + target->getWidget()->getSize().d_height * 0.5f;
        const float anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize.d_height);
        _scene.setAnchorValues(minX, maxX, anchorValue, anchorValue, true);
    }
    close();
}

void AnchorPopupMenu::on_btnSelfCenter_clicked()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto size = target->getWidget()->getSize();
        const auto pt = target->getWidget()->getPosition() + CEGUI::UVector2(0.5f * size.d_width, 0.5f * size.d_height);
        const auto anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize);
        _scene.setAnchorValues(anchorValue.x, anchorValue.x, anchorValue.y, anchorValue.y, true);
    }
    close();
}
