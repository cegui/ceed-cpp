#include "AnchorPopupMenu.h"
#include "ui_AnchorPopupMenu.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/Application.h"
#include "src/util/Settings.h"
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
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentTop_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.f, 0.f, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_btnParentLeftTop_clicked()
{
    _scene.setAnchorValues(0.f, 0.f, 0.f, 0.f, true);
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentHCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(0.5f, 0.5f, minY, maxY, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentVCenter_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.5f, 0.5f, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_btnParentCenter_clicked()
{
    _scene.setAnchorValues(0.5f, 0.5f, 0.5f, 0.5f, true);
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentRight_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(1.f, 1.f, minY, maxY, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentBottom_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 1.f, 1.f, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_btnParentRightBottom_clicked()
{
    _scene.setAnchorValues(1.f, 1.f, 1.f, 1.f, true);
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentHStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(0.f, 1.f, minY, maxY, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_actionParentVStretch_triggered()
{
    float minX, maxX, minY, maxY;
    if (_scene.getAnchorValues(minX, maxX, minY, maxY))
        _scene.setAnchorValues(minX, maxX, 0.f, 1.f, true);
    closeIfNeeded();
}

void AnchorPopupMenu::on_btnParentStretch_clicked()
{
    _scene.setAnchorValues(0.f, 1.f, 0.f, 1.f, true);
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
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
    closeIfNeeded();
}

void AnchorPopupMenu::closeIfNeeded()
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    if (settings->getEntryValue("layout/visual/close_anchor_presets").toBool())
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

    const QRectF parentRect = _scene.getAnchorTarget()->getParentSceneRect();
    const float toleranceX = 1.f / static_cast<float>(parentRect.width()) - std::numeric_limits<float>().epsilon();
    const float toleranceY = 1.f / static_cast<float>(parentRect.height()) - std::numeric_limits<float>().epsilon();

    if (compareFloat(minX, 0.f, toleranceX) && compareFloat(maxX, 0.f, toleranceX))
        ui->actionParentLeft->setChecked(true);
    else if (compareFloat(minX, 0.f, toleranceX) && compareFloat(maxX, 1.f, toleranceX))
        ui->actionParentHStretch->setChecked(true);
    else if (compareFloat(minX, 0.5f, toleranceX) && compareFloat(maxX, 0.5f, toleranceX))
        ui->actionParentHCenter->setChecked(true);
    else if (compareFloat(minX, 1.f, toleranceX) && compareFloat(maxX, 1.f, toleranceX))
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

        if (compareFloat(minX, minAnchorValue, toleranceX))
        {
            if (compareFloat(maxX, minAnchorValue, toleranceX))
                ui->actionSelfLeft->setChecked(true);
            else if (compareFloat(maxX, maxAnchorValue, toleranceX))
                ui->actionSelfHStretch->setChecked(true);
        }
        else if (compareFloat(minX, midAnchorValue, toleranceX) && compareFloat(maxX, midAnchorValue, toleranceX))
            ui->actionSelfHCenter->setChecked(true);
        else if (compareFloat(minX, maxAnchorValue, toleranceX) && compareFloat(maxX, maxAnchorValue, toleranceX))
            ui->actionSelfRight->setChecked(true);
    }

    if (compareFloat(minY, 0.f, toleranceY) && compareFloat(maxY, 0.f, toleranceY))
        ui->actionParentTop->setChecked(true);
    else if (compareFloat(minY, 0.f, toleranceY) && compareFloat(maxY, 1.f, toleranceY))
        ui->actionParentVStretch->setChecked(true);
    else if (compareFloat(minY, 0.5f, toleranceY) && compareFloat(maxY, 0.5f, toleranceY))
        ui->actionParentVCenter->setChecked(true);
    else if (compareFloat(minY, 1.f, toleranceY) && compareFloat(maxY, 1.f, toleranceY))
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

        if (compareFloat(minY, minAnchorValue, toleranceY))
        {
            if (compareFloat(maxY, minAnchorValue, toleranceY))
                ui->actionSelfTop->setChecked(true);
            else if (compareFloat(maxY, maxAnchorValue, toleranceY))
                ui->actionSelfVStretch->setChecked(true);
        }
        else if (compareFloat(minY, midAnchorValue, toleranceY) && compareFloat(maxY, midAnchorValue, toleranceY))
            ui->actionSelfVCenter->setChecked(true);
        else if (compareFloat(minY, maxAnchorValue, toleranceY) && compareFloat(maxY, maxAnchorValue, toleranceY))
            ui->actionSelfBottom->setChecked(true);
    }

    QWidget::showEvent(event);
}
