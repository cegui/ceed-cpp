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
    ui->btnParentRight->setDefaultAction(ui->actionParentRight);
    ui->btnSelfRight->setDefaultAction(ui->actionSelfRight);
    ui->btnParentBottom->setDefaultAction(ui->actionParentBottom);
    ui->btnSelfBottom->setDefaultAction(ui->actionSelfBottom);
    ui->btnParentHStretch->setDefaultAction(ui->actionParentHStretch);
    ui->btnSelfHStretch->setDefaultAction(ui->actionSelfHStretch);
    ui->btnParentVStretch->setDefaultAction(ui->actionParentVStretch);
    ui->btnSelfVStretch->setDefaultAction(ui->actionSelfVStretch);
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

void AnchorPopupMenu::on_actionParentRight_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(1.f, 1.f, minY, maxY, true);
    close();
}

void AnchorPopupMenu::on_actionParentBottom_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 1.f, 1.f, true);
    close();
}

void AnchorPopupMenu::on_btnParentRightBottom_clicked()
{
    _scene.setAnchorValues(1.f, 1.f, 1.f, 1.f, true);
    close();
}

void AnchorPopupMenu::on_actionSelfRight_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto pt = target->getWidget()->getPosition().d_x + target->getWidget()->getSize().d_width;
        const float anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize.d_width);
        _scene.setAnchorValues(anchorValue, anchorValue, minY, maxY, true);
    }
    close();
}

void AnchorPopupMenu::on_actionSelfBottom_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto pt = target->getWidget()->getPosition().d_y + target->getWidget()->getSize().d_height;
        const float anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize.d_height);
        _scene.setAnchorValues(minX, maxX, anchorValue, anchorValue, true);
    }
    close();
}

void AnchorPopupMenu::on_btnSelfRightBottom_clicked()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto size = target->getWidget()->getSize();
        const auto pt = target->getWidget()->getPosition() + CEGUI::UVector2(size.d_width, size.d_height);
        const auto anchorValue = CEGUI::CoordConverter::asRelative(pt, baseSize);
        _scene.setAnchorValues(anchorValue.x, anchorValue.x, anchorValue.y, anchorValue.y, true);
    }
    close();
}

void AnchorPopupMenu::on_actionParentHStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(0.f, 1.f, minY, maxY, true);
    close();
}

void AnchorPopupMenu::on_actionParentVStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.f, 1.f, true);
    close();
}

void AnchorPopupMenu::on_btnParentStretch_clicked()
{
    _scene.setAnchorValues(0.f, 1.f, 0.f, 1.f, true);
    close();
}

void AnchorPopupMenu::on_actionSelfHStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto minPt = target->getWidget()->getPosition().d_x;
        const auto maxPt = minPt + target->getWidget()->getSize().d_width;
        const float minAnchorValue = CEGUI::CoordConverter::asRelative(minPt, baseSize.d_width);
        const float maxAnchorValue = CEGUI::CoordConverter::asRelative(maxPt, baseSize.d_width);
        _scene.setAnchorValues(minAnchorValue, maxAnchorValue, minY, maxY, true);
    }
    close();
}

void AnchorPopupMenu::on_actionSelfVStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto minPt = target->getWidget()->getPosition().d_y;
        const auto maxPt = minPt + target->getWidget()->getSize().d_height;
        const float minAnchorValue = CEGUI::CoordConverter::asRelative(minPt, baseSize.d_height);
        const float maxAnchorValue = CEGUI::CoordConverter::asRelative(maxPt, baseSize.d_height);
        _scene.setAnchorValues(minX, maxX, minAnchorValue, maxAnchorValue, true);
    }
    close();
}

void AnchorPopupMenu::on_btnSelfStretch_clicked()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto size = target->getWidget()->getSize();
        const auto minPt = target->getWidget()->getPosition();
        const auto maxPt = minPt + CEGUI::UVector2(size.d_width, size.d_height);
        const auto minAnchorValue = CEGUI::CoordConverter::asRelative(minPt, baseSize);
        const auto maxAnchorValue = CEGUI::CoordConverter::asRelative(maxPt, baseSize);
        _scene.setAnchorValues(minAnchorValue.x, maxAnchorValue.x, minAnchorValue.y, maxAnchorValue.y, true);
    }
    close();
}
