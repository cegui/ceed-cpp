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

    _horzGroup = new QActionGroup(this);
    _horzGroup->addAction(ui->actionParentLeft);
    _horzGroup->addAction(ui->actionSelfLeft);
    _horzGroup->addAction(ui->actionParentHCenter);
    _horzGroup->addAction(ui->actionSelfHCenter);
    _horzGroup->addAction(ui->actionParentRight);
    _horzGroup->addAction(ui->actionSelfRight);
    _horzGroup->addAction(ui->actionParentHStretch);
    _horzGroup->addAction(ui->actionSelfHStretch);

    _vertGroup = new QActionGroup(this);
    _vertGroup->addAction(ui->actionParentTop);
    _vertGroup->addAction(ui->actionSelfTop);
    _vertGroup->addAction(ui->actionParentVCenter);
    _vertGroup->addAction(ui->actionSelfVCenter);
    _vertGroup->addAction(ui->actionParentBottom);
    _vertGroup->addAction(ui->actionSelfBottom);
    _vertGroup->addAction(ui->actionParentVStretch);
    _vertGroup->addAction(ui->actionSelfVStretch);

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

static inline bool compareFloat(float a, float b, float tolerance)
{
    return std::abs(a - b) <= tolerance;
}

// Detect current presets. Tolerance is relatively big because of offset pixel rounding,
// which may lead relative part to be not exactly the value requested.
void AnchorPopupMenu::showEvent(QShowEvent* event)
{
    for (auto action : _horzGroup->actions())
        action->setChecked(false);

    for (auto action : _vertGroup->actions())
        action->setChecked(false);

    float minX, maxX, minY, maxY;
    if (!_scene.getAnchorValues(minX, maxX, minY, maxY)) return;

    constexpr float tolerance = 1.f / 1920.f;

    if (compareFloat(minX, 0.f, tolerance) && compareFloat(maxX, 0.f, tolerance))
        ui->actionParentLeft->setChecked(true);
    else if (compareFloat(minX, 0.f, tolerance) && compareFloat(maxX, 1.f, tolerance))
        ui->actionParentHStretch->setChecked(true);
    else if (compareFloat(minX, 0.5f, tolerance) && compareFloat(maxX, 0.5f, tolerance))
        ui->actionParentHCenter->setChecked(true);
    else if (compareFloat(minX, 1.f, tolerance) && compareFloat(maxX, 1.f, tolerance))
        ui->actionParentRight->setChecked(true);
    else
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto minPt = target->getWidget()->getPosition().d_x;
        const auto midPt = minPt + target->getWidget()->getSize().d_width * 0.5f;
        const auto maxPt = minPt + target->getWidget()->getSize().d_width;
        const auto minAnchorValue = CEGUI::CoordConverter::asRelative(minPt, baseSize.d_width);
        const auto midAnchorValue = CEGUI::CoordConverter::asRelative(midPt, baseSize.d_width);
        const auto maxAnchorValue = CEGUI::CoordConverter::asRelative(maxPt, baseSize.d_width);

        if (compareFloat(minX, minAnchorValue, tolerance))
        {
            if (compareFloat(maxX, minAnchorValue, tolerance))
                ui->actionSelfLeft->setChecked(true);
            else if (compareFloat(maxX, maxAnchorValue, tolerance))
                ui->actionSelfHStretch->setChecked(true);
        }
        else if (compareFloat(minX, midAnchorValue, tolerance) && compareFloat(maxX, midAnchorValue, tolerance))
            ui->actionSelfHCenter->setChecked(true);
        else if (compareFloat(minX, maxAnchorValue, tolerance) && compareFloat(maxX, maxAnchorValue, tolerance))
            ui->actionSelfRight->setChecked(true);
    }

    if (compareFloat(minY, 0.f, tolerance) && compareFloat(maxY, 0.f, tolerance))
        ui->actionParentTop->setChecked(true);
    else if (compareFloat(minY, 0.f, tolerance) && compareFloat(maxY, 1.f, tolerance))
        ui->actionParentVStretch->setChecked(true);
    else if (compareFloat(minY, 0.5f, tolerance) && compareFloat(maxY, 0.5f, tolerance))
        ui->actionParentVCenter->setChecked(true);
    else if (compareFloat(minY, 1.f, tolerance) && compareFloat(maxY, 1.f, tolerance))
        ui->actionParentBottom->setChecked(true);
    else
    {
        LayoutManipulator* target = _scene.getAnchorTarget();
        const auto baseSize = target->getBaseSize();
        const auto minPt = target->getWidget()->getPosition().d_y;
        const auto midPt = minPt + target->getWidget()->getSize().d_height * 0.5f;
        const auto maxPt = minPt + target->getWidget()->getSize().d_height;
        const auto minAnchorValue = CEGUI::CoordConverter::asRelative(minPt, baseSize.d_height);
        const auto midAnchorValue = CEGUI::CoordConverter::asRelative(midPt, baseSize.d_height);
        const auto maxAnchorValue = CEGUI::CoordConverter::asRelative(maxPt, baseSize.d_height);

        if (compareFloat(minY, minAnchorValue, tolerance))
        {
            if (compareFloat(maxY, minAnchorValue, tolerance))
                ui->actionSelfTop->setChecked(true);
            else if (compareFloat(maxY, maxAnchorValue, tolerance))
                ui->actionSelfVStretch->setChecked(true);
        }
        else if (compareFloat(minY, midAnchorValue, tolerance) && compareFloat(maxY, midAnchorValue, tolerance))
            ui->actionSelfVCenter->setChecked(true);
        else if (compareFloat(minY, maxAnchorValue, tolerance) && compareFloat(maxY, maxAnchorValue, tolerance))
            ui->actionSelfBottom->setChecked(true);
    }

    QWidget::showEvent(event);
}
