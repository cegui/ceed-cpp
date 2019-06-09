#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/AnchorCornerHandle.h"
#include "src/ui/layout/AnchorEdgeHandle.h"
#include "src/ui/NumericValueItem.h"
#include "src/ui/ResizingHandle.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include <CEGUI/CoordConverter.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/widgets/SequentialLayoutContainer.h>
#include "qgraphicssceneevent.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qtreeview.h"
#include "qstandarditemmodel.h"
#include <set>

// For properties (may be incapsulated somewhere):
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include "PropertyWidget/PropertyWidget.h"

LayoutScene::LayoutScene(LayoutVisualMode& visualMode)
    : CEGUIGraphicsScene(&visualMode)
    , _visualMode(visualMode)
{
    connect(this, &LayoutScene::selectionChanged, this, &LayoutScene::slot_selectionChanged);
}

LayoutScene::~LayoutScene()
{
    disconnect(this, &LayoutScene::selectionChanged, this, &LayoutScene::slot_selectionChanged);
}

void LayoutScene::updateFromWidgets()
{
    if (rootManipulator) rootManipulator->updateFromWidget();
}

// Overridden to keep the manipulators in sync
void LayoutScene::setCEGUIDisplaySize(float width, float height)
{
    CEGUIGraphicsScene::setCEGUIDisplaySize(width, height);
    updateFromWidgets();
}

void LayoutScene::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
    if (_anchorTextX) disconnect(_anchorTextX, &NumericValueItem::valueChanged, nullptr, nullptr);
    if (_anchorTextY) disconnect(_anchorTextY, &NumericValueItem::valueChanged, nullptr, nullptr);

    _anchorTarget = nullptr;
    _anchorSnapTarget = nullptr;

    clear();

    rootManipulator = manipulator;

    if (rootManipulator)
    {
        ceguiContext->setRootWindow(rootManipulator->getWidget());

        // Root manipulator changed, perform a full update
        // NB: widget must be already set to a CEGUI context for area calculation
        rootManipulator->updateFromWidget(true);
        addItem(rootManipulator);

        createAnchorItems();
    }
    else
    {
        ceguiContext->setRootWindow(nullptr);

        _anchorParentRect = nullptr;
        _anchorMinX = nullptr;
        _anchorMinY = nullptr;
        _anchorMaxX = nullptr;
        _anchorMaxY = nullptr;
        _anchorMinXMinY = nullptr;
        _anchorMaxXMinY = nullptr;
        _anchorMinXMaxY = nullptr;
        _anchorMaxXMaxY = nullptr;
        _anchorTextX = nullptr;
        _anchorTextY = nullptr;
    }
}

LayoutManipulator* LayoutScene::getManipulatorByPath(const QString& widgetPath) const
{
    if (!rootManipulator) return nullptr;

    auto sepPos = widgetPath.indexOf('/');
    if (sepPos < 0)
    {
        assert(widgetPath == rootManipulator->getWidgetName());
        return rootManipulator;
    }
    else
    {
        assert(widgetPath.leftRef(sepPos) == rootManipulator->getWidgetName());
        return dynamic_cast<LayoutManipulator*>(rootManipulator->getManipulatorByPath(widgetPath.mid(sepPos + 1)));
    }
}

void LayoutScene::normalizePositionOfSelectedWidgets()
{
    std::vector<LayoutMoveCommand::Record> records;
    bool toRelative = false;

    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        const auto pos = manipulator->getWidget()->getPosition();

        LayoutMoveCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = pos;
        records.push_back(std::move(rec));

        // For absolute-relative cycling
        if (!toRelative && (pos.d_x.d_offset != 0.f || pos.d_y.d_offset != 0.f))
            toRelative = true;
    }

    if (records.empty()) return;

    for (auto& rec : records)
    {
        const LayoutManipulator* manipulator = getManipulatorByPath(rec.path);
        const auto baseSize = manipulator->getBaseSize();

        if (toRelative) // command id was base + 12
        {
            rec.newPos.d_x.d_scale = (rec.oldPos.d_x.d_offset + rec.oldPos.d_x.d_scale * baseSize.d_width) / baseSize.d_width;
            rec.newPos.d_x.d_offset = 0.f;
            rec.newPos.d_y.d_scale = (rec.oldPos.d_y.d_offset + rec.oldPos.d_y.d_scale * baseSize.d_height) / baseSize.d_height;
            rec.newPos.d_y.d_offset = 0.f;
        }
        else // command id was base + 13
        {
            rec.newPos.d_x.d_scale = 0.f;
            rec.newPos.d_x.d_offset = rec.oldPos.d_x.d_offset + rec.oldPos.d_x.d_scale * baseSize.d_width;
            rec.newPos.d_y.d_scale = 0.f;
            rec.newPos.d_y.d_offset = rec.oldPos.d_y.d_offset + rec.oldPos.d_y.d_scale * baseSize.d_height;
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
}

void LayoutScene::normalizeSizeOfSelectedWidgets()
{
    std::vector<LayoutResizeCommand::Record> records;
    bool toRelative = false;

    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        const auto size = manipulator->getWidget()->getSize();

        LayoutResizeCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = manipulator->getWidget()->getPosition();
        rec.oldSize = size;
        rec.newPos = rec.oldPos;
        records.push_back(std::move(rec));

        // For absolute-relative cycling
        if (!toRelative && (size.d_width.d_offset != 0.f || size.d_height.d_offset != 0.f))
            toRelative = true;
    }

    if (records.empty()) return;

    for (auto& rec : records)
    {
        const LayoutManipulator* manipulator = getManipulatorByPath(rec.path);
        const auto pixelSize = manipulator->getWidget()->getPixelSize();

        if (toRelative) // command id was base + 10
        {
            const auto baseSize = manipulator->getBaseSize();
            rec.newSize.d_width.d_scale = pixelSize.d_width / baseSize.d_width;
            rec.newSize.d_width.d_offset = 0.f;
            rec.newSize.d_height.d_scale = pixelSize.d_height / baseSize.d_height;
            rec.newSize.d_height.d_offset = 0.f;
        }
        else // command id was base + 11
        {
            rec.newSize.d_width.d_scale = 0.f;
            rec.newSize.d_width.d_offset = pixelSize.d_width;
            rec.newSize.d_height.d_scale = 0.f;
            rec.newSize.d_height.d_offset = pixelSize.d_height;
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(records)));
}

// command id was base + 15
void LayoutScene::roundPositionOfSelectedWidgets()
{
    std::vector<LayoutMoveCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutMoveCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldPos = manipulator->getWidget()->getPosition();
            rec.newPos = rec.oldPos;
            rec.newPos.d_x.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_x.d_offset);
            rec.newPos.d_y.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_y.d_offset);
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
}

void LayoutScene::roundSizeOfSelectedWidgets()
{
    std::vector<LayoutResizeCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutResizeCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldPos = manipulator->getWidget()->getPosition();
            rec.newPos = rec.oldPos;
            rec.oldSize = manipulator->getWidget()->getSize();
            rec.newSize = rec.oldSize;
            rec.newSize.d_width.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldSize.d_width.d_offset);
            rec.newSize.d_height.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldSize.d_height.d_offset);
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(records)));
}

void LayoutScene::alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment)
{
    std::vector<LayoutHorizontalAlignCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutHorizontalAlignCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldAlignment = manipulator->getWidget()->getHorizontalAlignment();
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutHorizontalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::alignSelectionVertically(CEGUI::VerticalAlignment alignment)
{
    std::vector<LayoutVerticalAlignCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutVerticalAlignCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldAlignment = manipulator->getWidget()->getVerticalAlignment();
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutVerticalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::moveSelectedWidgetsInParentWidgetLists(int delta)
{
    QStringList paths;
    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        auto parentManipulator = dynamic_cast<LayoutManipulator*>(item->parentItem());
        if (!parentManipulator) continue;

        auto container = dynamic_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->getWidget());
        if (!container) continue;

        int potentialPos = static_cast<int>(container->getPositionOfChild(manipulator->getWidget())) + delta;
        if (potentialPos < 0 || potentialPos >= static_cast<int>(container->getChildCount())) continue;

        paths.append(manipulator->getWidgetPath());
    }

    // TODO: We currently only support moving one widget at a time.
    //       Fixing this involves sorting the widgets by their position in
    //       the parent widget and then either working from the "right" side
    //       if delta > 0 or from the left side if delta < 0.
    if (paths.size() == 1)
        _visualMode.getEditor().getUndoStack()->push(new MoveInParentWidgetListCommand(_visualMode, std::move(paths), delta));
}

bool LayoutScene::deleteSelectedWidgets()
{
    QStringList widgetPaths;
    for (auto& item : selectedItems())
    {
        auto manip = dynamic_cast<LayoutManipulator*>(item);
        if (manip) widgetPaths.push_back(manip->getWidgetPath());
    }

    if (!widgetPaths.isEmpty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutDeleteCommand(_visualMode, std::move(widgetPaths)));

    return true;
}

void LayoutScene::onManipulatorUpdatedFromWidget(LayoutManipulator* manipulator)
{
    if (!manipulator) return;

    // Update anchor handles for the manipulator if they are shown,
    // unless updateFromWidget() was called due to dragging them
    if (_anchorTarget == manipulator)
    {
        const bool isAnchorHandleSelected = isAnyAnchorHandleSelected();
        if (!_anchorTarget->resizeInProgress() || !isAnchorHandleSelected)
            updateAnchorItems();

        if (!isAnchorHandleSelected)
        {
            _anchorTextX->setVisible(false);
            _anchorTextY->setVisible(false);
        }
    }
}

static void ensureParentIsExpanded(QTreeView* view, QStandardItem* treeItem)
{
    view->expand(treeItem->index());
    if (treeItem->parent())
        ensureParentIsExpanded(view, treeItem->parent());
}

void LayoutScene::slot_selectionChanged()
{
    // Collect selected widgets and anchor items

    std::set<LayoutManipulator*> selectedWidgets;
    QGraphicsItem* selectedAnchorItem = nullptr;

    auto selection = selectedItems();
    for (QGraphicsItem* item : selection)
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            selectedWidgets.insert(manipulator);
        }
        else if (dynamic_cast<ResizingHandle*>(item))
        {
            if (auto manipulator = dynamic_cast<LayoutManipulator*>(item->parentItem()))
                selectedWidgets.insert(manipulator);
        }
        else if (item == _anchorMinX || item == _anchorMinY || item == _anchorMaxX || item == _anchorMaxY ||
                 item == _anchorMinXMinY || item == _anchorMaxXMinY || item == _anchorMinXMaxY || item == _anchorMaxXMaxY)
        {
            // Only one selected anchor item at a time is allowed
            assert(!selectedAnchorItem);
            selectedAnchorItem = item;
        }
    }

    // Update anchors state

    if (selectedWidgets.size() > 1)
    {
        // Hide anchors if we selected multiple widgets
        _anchorTarget = nullptr;
    }
    else if (selectedWidgets.size() == 0)
    {
        // If we selected anchor item, we therefore deselected an _anchorTarget,
        // but it must look as selected in a GUI so we don't change anything
        if (selectedAnchorItem) return;

        // Nothing interesting is selected, hide anchors
        _anchorTarget = nullptr;
    }
    else
    {
        // Show anchors for the only selected widget
        if (_anchorTarget == *selectedWidgets.begin()) return;
        _anchorTarget = *selectedWidgets.begin();
    }

    _anchorTextX->setVisible(false);
    _anchorTextY->setVisible(false);

    updateAnchorItems();

    // Update property view for our selection

    // TODO: to method (whose?)
    QtnPropertySet* propertySet = nullptr;
    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        // TODO: create multi property set
        //!!!pass vector of sources & combine inside an entry point method!
        propertySet = manipulator->getPropertySet();
    }
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    auto propertyWidget = static_cast<QtnPropertyWidget*>(mainWindow->getPropertyDockWidget()->widget());
    propertyWidget->setPropertySet(propertySet);

    // Show selection in a hierarchy tree

    if (!_ignoreSelectionChanges)
    {
        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(true);

        _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

        auto treeView = _visualMode.getHierarchyDockWidget()->getTreeView();

        QStandardItem* lastTreeItem = nullptr;
        for (QGraphicsItem* item : selection)
        {
            auto manip = dynamic_cast<LayoutManipulator*>(item);
            if (manip && manip->getTreeItem())
            {
                treeView->selectionModel()->select(manip->getTreeItem()->index(), QItemSelectionModel::Select);
                ensureParentIsExpanded(treeView, manip->getTreeItem());
                lastTreeItem = manip->getTreeItem();
            }
        }

        if (lastTreeItem) treeView->scrollTo(lastTreeItem->index());

        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(false);
    }
}

void LayoutScene::createAnchorItems()
{
    QPen anchorRectPen(Qt::PenStyle::SolidLine);
    anchorRectPen.setColor(Qt::magenta);
    anchorRectPen.setWidth(2);
    anchorRectPen.setCosmetic(true);

    _anchorParentRect = new QGraphicsRectItem();
    _anchorParentRect->setVisible(false);
    _anchorParentRect->setPen(anchorRectPen);
    addItem(_anchorParentRect);

    QPen anchorGuidePen(Qt::PenStyle::CustomDashLine);
    anchorGuidePen.setDashPattern({ 8.0, 4.0 });
    anchorGuidePen.setColor(Qt::white);
    anchorGuidePen.setWidth(2);
    anchorGuidePen.setCosmetic(true);

    _anchorMinX = new AnchorEdgeHandle(false, nullptr, anchorGuidePen, Qt::cyan);
    _anchorMinX->setVisible(false);
    addItem(_anchorMinX);

    _anchorMinY = new AnchorEdgeHandle(true, nullptr, anchorGuidePen, Qt::cyan);
    _anchorMinY->setVisible(false);
    addItem(_anchorMinY);

    _anchorMaxX = new AnchorEdgeHandle(false, nullptr, anchorGuidePen, Qt::cyan);
    _anchorMaxX->setVisible(false);
    addItem(_anchorMaxX);

    _anchorMaxY = new AnchorEdgeHandle(true, nullptr, anchorGuidePen, Qt::cyan);
    _anchorMaxY->setVisible(false);
    addItem(_anchorMaxY);

    QPen anchorCornerPen(Qt::PenStyle::SolidLine);
    anchorCornerPen.setColor(Qt::white);
    anchorCornerPen.setWidth(1);
    anchorCornerPen.setCosmetic(true);

    _anchorMinXMinY = new AnchorCornerHandle(true, true, nullptr, 16.0, anchorCornerPen, Qt::cyan);
    _anchorMinXMinY->setVisible(false);
    addItem(_anchorMinXMinY);

    _anchorMaxXMinY = new AnchorCornerHandle(false, true, nullptr, 16.0, anchorCornerPen, Qt::cyan);
    _anchorMaxXMinY->setVisible(false);
    addItem(_anchorMaxXMinY);

    _anchorMinXMaxY = new AnchorCornerHandle(true, false, nullptr, 16.0, anchorCornerPen, Qt::cyan);
    _anchorMinXMaxY->setVisible(false);
    addItem(_anchorMinXMaxY);

    _anchorMaxXMaxY = new AnchorCornerHandle(false, false, nullptr, 16.0, anchorCornerPen, Qt::cyan);
    _anchorMaxXMaxY->setVisible(false);
    addItem(_anchorMaxXMaxY);

    QFont anchorFont("Arial", 10, QFont::Bold);

    _anchorTextX = new NumericValueItem();
    _anchorTextX->setFont(anchorFont);
    _anchorTextX->setDefaultTextColor(Qt::white);
    _anchorTextX->setToolTip("Click to edit");
    _anchorTextX->setTextTemplate("Value: %1%");
    _anchorTextX->setHorizontalAlignment(Qt::AlignCenter);
    _anchorTextX->setPrecision(2);
    _anchorTextX->setVisible(false);
    addItem(_anchorTextX);

    _anchorTextY = new NumericValueItem();
    _anchorTextY->setFont(anchorFont);
    _anchorTextY->setDefaultTextColor(Qt::white);
    _anchorTextY->setToolTip("Click to edit");
    _anchorTextY->setTextTemplate("Value: %1%");
    _anchorTextY->setHorizontalAlignment(Qt::AlignCenter);
    _anchorTextY->setPrecision(2);
    _anchorTextY->setVisible(false);
    addItem(_anchorTextY);
}

bool LayoutScene::isAnyAnchorHandleSelected() const
{
    return _anchorMinX->isSelected() ||
            _anchorMaxX->isSelected() ||
            _anchorMinY->isSelected() ||
            _anchorMaxY->isSelected() ||
            _anchorMinXMinY->isSelected() ||
            _anchorMaxXMinY->isSelected() ||
            _anchorMinXMaxY->isSelected() ||
            _anchorMaxXMaxY->isSelected();
}

void LayoutScene::updateAnchorItems(QGraphicsItem* movedItem)
{
    // Too early call, items aren't created yet
    if (!_anchorTextX) return;

    const bool showAnchors = (_anchorTarget != nullptr);

    if (_anchorParentRect)
    {
        _anchorParentRect->setVisible(showAnchors);
        _anchorMinX->setVisible(showAnchors);
        _anchorMinY->setVisible(showAnchors);
        _anchorMaxX->setVisible(showAnchors);
        _anchorMaxY->setVisible(showAnchors);
        _anchorMinXMinY->setVisible(showAnchors);
        _anchorMaxXMinY->setVisible(showAnchors);
        _anchorMinXMaxY->setVisible(showAnchors);
        _anchorMaxXMaxY->setVisible(showAnchors);
    }

    if (!showAnchors || !_anchorParentRect)
    {
        _anchorTextX->setVisible(false);
        _anchorTextY->setVisible(false);

        _anchorMinX->setSelected(false);
        _anchorMinY->setSelected(false);
        _anchorMaxX->setSelected(false);
        _anchorMaxY->setSelected(false);
        _anchorMinXMinY->setSelected(false);
        _anchorMaxXMinY->setSelected(false);
        _anchorMinXMaxY->setSelected(false);
        _anchorMaxXMaxY->setSelected(false);
        _anchorTextX->setSelected(false);
        _anchorTextY->setSelected(false);

        return;
    }

    const QRectF parentRect = _anchorTarget->getParentRect();
    const QRectF anchorsRect = _anchorTarget->getAnchorsRect();

    _anchorParentRect->setRect(parentRect);

    // Position handles without firing move events
    if (movedItem != _anchorMinX) _anchorMinX->setPosSilent(anchorsRect.left(), 0.0);
    if (movedItem != _anchorMaxX) _anchorMaxX->setPosSilent(anchorsRect.right(), 0.0);
    if (movedItem != _anchorMinY) _anchorMinY->setPosSilent(0.0, anchorsRect.top());
    if (movedItem != _anchorMaxY) _anchorMaxY->setPosSilent(0.0, anchorsRect.bottom());
    if (movedItem != _anchorMinXMinY) _anchorMinXMinY->setPosSilent(anchorsRect.left(), anchorsRect.top());
    if (movedItem != _anchorMaxXMinY) _anchorMaxXMinY->setPosSilent(anchorsRect.right(), anchorsRect.top());
    if (movedItem != _anchorMinXMaxY) _anchorMinXMaxY->setPosSilent(anchorsRect.left(), anchorsRect.bottom());
    if (movedItem != _anchorMaxXMaxY) _anchorMaxXMaxY->setPosSilent(anchorsRect.right(), anchorsRect.bottom());
}

// Updates position and value of anchor texts corresponding to the anchor item passed
void LayoutScene::updateAnchorValueItems(bool minX, bool maxX, bool minY, bool maxY)
{
    if (!_anchorTarget) return;

    const auto widgetCenter = _anchorTarget->sceneBoundingRect().center();
    const auto& ceguiPos = _anchorTarget->getWidget()->getPosition();
    const auto& ceguiSize = _anchorTarget->getWidget()->getSize();

    if (minX)
    {
        _anchorTextX->setValue(static_cast<qreal>(ceguiPos.d_x.d_scale) * 100.0);
        _anchorTextX->setX(_anchorMinX->sceneBoundingRect().left() - _anchorTextX->sceneBoundingRect().width());// - 10);
        _anchorTextX->setY(widgetCenter.y() - _anchorTextX->sceneBoundingRect().height() / 2.0);
    }
    else if (maxX)
    {
        _anchorTextX->setValue(static_cast<qreal>(ceguiPos.d_x.d_scale + ceguiSize.d_width.d_scale) * 100.0);
        _anchorTextX->setX(_anchorMaxX->sceneBoundingRect().right());// + 10);
        _anchorTextX->setY(widgetCenter.y() - _anchorTextX->sceneBoundingRect().height() / 2.0);
    }

    if (minY)
    {
        _anchorTextY->setValue(static_cast<qreal>(ceguiPos.d_y.d_scale) * 100.0);
        _anchorTextY->setX(widgetCenter.x() - _anchorTextY->sceneBoundingRect().width() / 2.0);
        _anchorTextY->setY(_anchorMinY->sceneBoundingRect().top() - _anchorTextY->sceneBoundingRect().height()); // - 10
    }
    else if (maxY)
    {
        _anchorTextY->setValue(static_cast<qreal>(ceguiPos.d_y.d_scale + ceguiSize.d_height.d_scale) * 100.0);
        _anchorTextY->setX(widgetCenter.x() - _anchorTextY->sceneBoundingRect().width() / 2.0);
        _anchorTextY->setY(_anchorMaxY->sceneBoundingRect().bottom()); // + 10
    }
}

void LayoutScene::applyAnchorDeltas(float deltaMinX, float deltaMaxX, float deltaMinY, float deltaMaxY, bool preserveEffectiveSize)
{
    assert(_anchorTarget);

    CEGUI::Window* widget = _anchorTarget->getWidget();
    if (!widget) return;

    auto baseSize = _anchorTarget->getBaseSize();
    if (baseSize.d_width <= 0.f || baseSize.d_height <= 0.f) return;

    CEGUI::UVector2 deltaPos;
    CEGUI::USize deltaSize;

    const float reDeltaMinX = deltaMinX / baseSize.d_width;
    deltaPos.d_x.d_scale += reDeltaMinX;
    deltaSize.d_width.d_scale += -reDeltaMinX;
    if (preserveEffectiveSize)
    {
        deltaPos.d_x.d_offset += -deltaMinX;
        deltaSize.d_width.d_offset += deltaMinX;
    }

    const float reDeltaMinY = deltaMinY / baseSize.d_height;
    deltaPos.d_y.d_scale += reDeltaMinY;
    deltaSize.d_height.d_scale += -reDeltaMinY;
    if (preserveEffectiveSize)
    {
        deltaPos.d_y.d_offset += -deltaMinY;
        deltaSize.d_height.d_offset += deltaMinY;
    }

    const float reDeltaMaxX = deltaMaxX / baseSize.d_width;
    deltaSize.d_width.d_scale += reDeltaMaxX;
    if (preserveEffectiveSize)
    {
        deltaSize.d_width.d_offset += -deltaMaxX;
    }

    const float reDeltaMaxY = deltaMaxY / baseSize.d_height;
    deltaSize.d_height.d_scale += reDeltaMaxY;
    if (preserveEffectiveSize)
    {
        deltaSize.d_height.d_offset += -deltaMaxY;
    }

    // Because the Qt manipulator is always top left aligned in the CEGUI sense,
    // we have to process the size to factor in alignments if they differ
    switch (widget->getHorizontalAlignment())
    {
        case CEGUI::HorizontalAlignment::Centre:
            deltaPos.d_x += CEGUI::UDim(0.5f, 0.5f) * deltaSize.d_width; break;
        case CEGUI::HorizontalAlignment::Right:
            deltaPos.d_x += deltaSize.d_width; break;
        default: break;
    }
    switch (widget->getVerticalAlignment())
    {
        case CEGUI::VerticalAlignment::Centre:
            deltaPos.d_y += CEGUI::UDim(0.5f, 0.5f) * deltaSize.d_height; break;
        case CEGUI::VerticalAlignment::Bottom:
            deltaPos.d_y += deltaSize.d_height; break;
        default: break;
    }

    widget->setPosition(widget->getPosition() + deltaPos);
    widget->setSize(widget->getSize() + deltaSize);

    _anchorTarget->updateFromWidget();
    _anchorTarget->updatePropertiesFromWidget({"Size", "Position", "Area"});
}

//!!!FIXME: working with deltas may lead to error accumulation!
// TODO: Lock axis
// TODO: presets in virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override?
// TODO: create undo command when edited through numeric value items or presets!
//???TODO: unify area change code with resizing through rect?
void LayoutScene::anchorHandleMoved(QGraphicsItem* item, QPointF& delta, bool moveOpposite)
{
    if (!_anchorTarget || !item) return;

    CEGUI::Window* widget = _anchorTarget->getWidget();
    if (!widget) return;

    if (!_anchorTarget->resizeInProgress())
        _anchorTarget->beginResizing(*item);

    // Snap to siblings (before limiting to ensure that limits are respected)

    //???snap corners too? if yes, remove this condition and implement!
    if (item == _anchorMinX || item == _anchorMaxX || item == _anchorMinY || item == _anchorMaxY)
    {
        // TODO: setting!
        constexpr qreal snapDistance = 10.0;

        bool snapped = false;
        const auto& siblings = _anchorTarget->parentItem() ? _anchorTarget->parentItem()->childItems() : topLevelItems();
        for (QGraphicsItem* sibling : siblings)
        {
            const bool isSelf = (_anchorTarget == sibling);
            auto siblingManipulator = dynamic_cast<LayoutManipulator*>(sibling);
            if (!siblingManipulator) continue;

            const auto siblingRect = siblingManipulator->sceneBoundingRect();
            const auto siblingAnchorsRect = siblingManipulator->getAnchorsRect();
            auto anchorPos = item->pos() + delta;

            if (item == _anchorMinX || item == _anchorMaxX)
            {
                // Snap only when we are about to intersect a sibling's edge
                if (_lastCursorPos.y() < siblingRect.top() || _lastCursorPos.y() > siblingRect.bottom()) continue;

                if (std::abs(anchorPos.x() - siblingRect.left()) <= snapDistance)
                    anchorPos.rx() = siblingRect.left();
                else if (std::abs(anchorPos.x() - siblingRect.right()) <= snapDistance)
                    anchorPos.rx() = siblingRect.right();
                else if (!isSelf && std::abs(anchorPos.x() - siblingAnchorsRect.left()) <= snapDistance)
                    anchorPos.rx() = siblingAnchorsRect.left();
                else if (!isSelf && std::abs(anchorPos.x() - siblingAnchorsRect.right()) <= snapDistance)
                    anchorPos.rx() = siblingAnchorsRect.right();
                else continue;

                delta = anchorPos - item->pos();
            }
            else if (item == _anchorMinY || item == _anchorMaxY)
            {
                // Snap only when we are about to intersect a sibling's edge
                if (_lastCursorPos.x() < siblingRect.left() || _lastCursorPos.x() > siblingRect.right()) continue;

                if (std::abs(anchorPos.y() - siblingRect.top()) <= snapDistance)
                    anchorPos.ry() = siblingRect.top();
                else if (std::abs(anchorPos.y() - siblingRect.bottom()) <= snapDistance)
                    anchorPos.ry() = siblingRect.bottom();
                else if (!isSelf && std::abs(anchorPos.y() - siblingAnchorsRect.top()) <= snapDistance)
                    anchorPos.ry() = siblingAnchorsRect.top();
                else if (!isSelf && std::abs(anchorPos.y() - siblingAnchorsRect.bottom()) <= snapDistance)
                    anchorPos.ry() = siblingAnchorsRect.bottom();
                else continue;

                delta = anchorPos - item->pos();
            }
            else continue;

            //???change color of snapped guide? item->setSnapped(true); , color in constructor. May help
            // visualizing snapping to anchor rect of the sibling without drawing it! Or render siblingAnchorsRect?
            if (_anchorSnapTarget != siblingManipulator)
            {
                if (_anchorSnapTarget) _anchorSnapTarget->resetPen();
                _anchorSnapTarget = siblingManipulator;
                _anchorSnapTarget->setPen(QColor(Qt::magenta));
            }
            snapped = true;

            // Reaching this means we snapped to the current sibling, skip others
            break;
        }

        if (!snapped && _anchorSnapTarget)
        {
            _anchorSnapTarget->resetPen();
            _anchorSnapTarget = nullptr;
        }
    }

    // Calculate actual deltas for anchors. Do limiting on a pixel level, it is more convenient.

    float deltaMinX = 0.f;
    float deltaMinY = 0.f;
    float deltaMaxX = 0.f;
    float deltaMaxY = 0.f;

    const QPointF newItemPos = item->pos() + delta;

    bool minX = false, maxX = false, minY = false, maxY = false;
    if (item == _anchorMinX || item == _anchorMinXMinY || item == _anchorMinXMaxY)
    {
        minX = true;
        const qreal overlap = newItemPos.x() - _anchorMaxX->pos().x();
        if (overlap > 0.0)
        {
            if (moveOpposite) deltaMaxX += static_cast<float>(overlap);
            else delta.rx() -= overlap;
        }

        if (_visualMode.isAbsoluteIntegerMode())
            delta.rx() = std::floor(delta.x());

        deltaMinX += static_cast<float>(delta.x());
    }
    else if (item == _anchorMaxX || item == _anchorMaxXMinY || item == _anchorMaxXMaxY)
    {
        maxX = true;
        const qreal overlap = newItemPos.x() - _anchorMinX->pos().x();
        if (overlap < 0.0)
        {
            if (moveOpposite) deltaMinX += static_cast<float>(overlap);
            else delta.rx() -= overlap;
        }

        if (_visualMode.isAbsoluteIntegerMode())
            delta.rx() = std::floor(delta.x());

        deltaMaxX += static_cast<float>(delta.x());
    }

    if (item == _anchorMinY || item == _anchorMinXMinY || item == _anchorMaxXMinY)
    {
        minY = true;
        const qreal overlap = newItemPos.y() - _anchorMaxY->pos().y();
        if (overlap > 0.0)
        {
            if (moveOpposite) deltaMaxY += static_cast<float>(overlap);
            else delta.ry() -= overlap;
        }

        if (_visualMode.isAbsoluteIntegerMode())
            delta.ry() = std::floor(delta.y());

        deltaMinY += static_cast<float>(delta.y());
    }
    else if (item == _anchorMaxY || item == _anchorMinXMaxY || item == _anchorMaxXMaxY)
    {
        maxY = true;
        const qreal overlap = newItemPos.y() - _anchorMinY->pos().y();
        if (overlap < 0.0)
        {
            if (moveOpposite) deltaMinY += static_cast<float>(overlap);
            else delta.ry() -= overlap;
        }

        if (_visualMode.isAbsoluteIntegerMode())
            delta.ry() = std::floor(delta.y());

        deltaMaxY += static_cast<float>(delta.y());
    }

    // Perform actual changes

    const bool preserveEffectiveSize = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
    applyAnchorDeltas(deltaMinX, deltaMaxX, deltaMinY, deltaMaxY, preserveEffectiveSize);

    updateAnchorItems(item);
    updateAnchorValueItems(minX, maxX, minY, maxY);
}

void LayoutScene::anchorHandleSelected(QGraphicsItem* item)
{
    // Only one anchor handle may be selected at a time
    if (item != _anchorMinX) _anchorMinX->setSelected(false);
    if (item != _anchorMinY) _anchorMinY->setSelected(false);
    if (item != _anchorMaxX) _anchorMaxX->setSelected(false);
    if (item != _anchorMaxY) _anchorMaxY->setSelected(false);
    if (item != _anchorMinXMinY) _anchorMinXMinY->setSelected(false);
    if (item != _anchorMaxXMinY) _anchorMaxXMinY->setSelected(false);
    if (item != _anchorMinXMaxY) _anchorMinXMaxY->setSelected(false);
    if (item != _anchorMaxXMaxY) _anchorMaxXMaxY->setSelected(false);

    // Show editable values for the handle selected

    disconnect(_anchorTextX, &NumericValueItem::valueChanged, nullptr, nullptr);
    disconnect(_anchorTextY, &NumericValueItem::valueChanged, nullptr, nullptr);

    if (!_anchorTarget)
    {
        _anchorTextX->setVisible(false);
        _anchorTextY->setVisible(false);
        return;
    }

    bool minX = false, maxX = false, minY = false, maxY = false;
    if (item == _anchorMinX || item == _anchorMinXMinY || item == _anchorMinXMaxY)
    {
        minX = true;
        _anchorTextX->setVisible(true);
        _anchorTextX->setHorizontalAlignment(Qt::AlignRight);
        _anchorTextX->setTextTemplate("%1%");
        connect(_anchorTextX, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minX = _anchorTarget->getWidget()->getPosition().d_x.d_scale;
            const float maxX = minX + _anchorTarget->getWidget()->getSize().d_width.d_scale;
            const float deltaMinX = static_cast<float>(newValue) / 100.f - minX;
            const float deltaMaxX = std::max(0.f, (minX + deltaMinX) - maxX);
            const float baseWidth = _anchorTarget->getBaseSize().d_width;
            applyAnchorDeltas(deltaMinX * baseWidth, deltaMaxX * baseWidth, 0.f, 0.f, false);
            updateAnchorItems();
            updateAnchorValueItems(true, false, false, false);
        });
     }
    else if (item == _anchorMaxX || item == _anchorMaxXMinY || item == _anchorMaxXMaxY)
    {
        maxX = true;
        _anchorTextX->setVisible(true);
        _anchorTextX->setHorizontalAlignment(Qt::AlignLeft);
        _anchorTextX->setTextTemplate("%1%");
        connect(_anchorTextX, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minX = _anchorTarget->getWidget()->getPosition().d_x.d_scale;
            const float maxX = minX + _anchorTarget->getWidget()->getSize().d_width.d_scale;
            const float deltaMaxX = static_cast<float>(newValue) / 100.f - maxX;
            const float deltaMinX = std::min(0.f, (maxX + deltaMaxX) - minX);
            const float baseWidth = _anchorTarget->getBaseSize().d_width;
            applyAnchorDeltas(deltaMinX * baseWidth, deltaMaxX * baseWidth, 0.f, 0.f, false);
            updateAnchorItems();
            updateAnchorValueItems(false, true, false, false);
        });
    }
    else _anchorTextX->setVisible(false);

    if (item == _anchorMinY || item == _anchorMinXMinY || item == _anchorMaxXMinY)
    {
        minY = true;
        _anchorTextY->setVisible(true);
        _anchorTextY->setTextTemplate("%1%");
        connect(_anchorTextY, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minY = _anchorTarget->getWidget()->getPosition().d_y.d_scale;
            const float maxY = minY + _anchorTarget->getWidget()->getSize().d_height.d_scale;
            const float deltaMinY = static_cast<float>(newValue) / 100.f - minY;
            const float deltaMaxY = std::max(0.f, (minY + deltaMinY) - maxY);
            const float baseHeight = _anchorTarget->getBaseSize().d_height;
            applyAnchorDeltas(0.f, 0.f, deltaMinY * baseHeight, deltaMaxY * baseHeight, false);
            updateAnchorItems();
            updateAnchorValueItems(false, false, true, false);
        });
    }
    else if (item == _anchorMaxY || item == _anchorMinXMaxY || item == _anchorMaxXMaxY)
    {
        maxY = true;
        _anchorTextY->setVisible(true);
        _anchorTextY->setTextTemplate("%1%");
        connect(_anchorTextY, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minY = _anchorTarget->getWidget()->getPosition().d_y.d_scale;
            const float maxY = minY + _anchorTarget->getWidget()->getSize().d_height.d_scale;
            const float deltaMaxY = static_cast<float>(newValue) / 100.f - maxY;
            const float deltaMinY = std::min(0.f, (maxY + deltaMaxY) - minY);
            const float baseHeight = _anchorTarget->getBaseSize().d_height;
            applyAnchorDeltas(0.f, 0.f, deltaMinY * baseHeight, deltaMaxY * baseHeight, false);
            updateAnchorItems();
            updateAnchorValueItems(false, false, false, true);
        });
    }
    else _anchorTextY->setVisible(false);

    updateAnchorValueItems(minX, maxX, minY, maxY);
}

void LayoutScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragEnterEvent(event);
    else
    {
        // Otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
            event->acceptProposedAction();
    }
}

void LayoutScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragEnterEvent(event);
}

void LayoutScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragMoveEvent(event);
    else
    {
        // Otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
            event->acceptProposedAction();
    }
}

void LayoutScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dropEvent(event);
    else
    {
        auto data = event->mimeData()->data("application/x-ceed-widget-type");
        if (data.size() > 0)
        {
            QString widgetType = data.data();
            int sepPos = widgetType.lastIndexOf('/');
            QString widgetName = (sepPos < 0) ? widgetType : widgetType.mid(sepPos + 1);
            _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, "", widgetType, widgetName));

            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
        }
    }
}

void LayoutScene::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    if (event->key() == Qt::Key_Delete)
    {
        handled = deleteSelectedWidgets();
    }

    if (handled)
        event->accept();
    else
        CEGUIGraphicsScene::keyReleaseEvent(event);
}

void LayoutScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    _lastCursorPos = event->lastScenePos();
    CEGUIGraphicsScene::mouseMoveEvent(event);
}

void LayoutScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    CEGUIGraphicsScene::mouseReleaseEvent(event);

    if (_anchorSnapTarget)
    {
        _anchorSnapTarget->resetPen();
        _anchorSnapTarget = nullptr;
    }

    // We have to "expand" the items, adding parents of resizing handles instead of the handles themselves
    std::set<LayoutManipulator*> selection;
    for (QGraphicsItem* selectedItem : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(selectedItem))
        {
            selection.insert(manipulator);
            continue;
        }

        if (auto handle = dynamic_cast<ResizingHandle*>(selectedItem))
        {
            if (auto manipulator = dynamic_cast<LayoutManipulator*>(handle->parentItem()))
            {
                selection.insert(manipulator);
                continue;
            }
        }
    }

    // Detect resizing by anchors not through selected items but through
    // the state of the anchored item itself
    if (_anchorTarget && _anchorTarget->resizeInProgress())
    {
        _anchorTarget->endResizing();
        selection.insert(_anchorTarget);
    }

    std::vector<LayoutMoveCommand::Record> move;
    std::vector<LayoutResizeCommand::Record> resize;

    for (LayoutManipulator* item : selection)
    {
        if (item->isMoveStarted())
        {
            LayoutMoveCommand::Record rec;
            rec.path = item->getWidgetPath();
            rec.oldPos = item->getStartPosition();
            rec.newPos = item->getWidget()->getPosition();
            move.push_back(std::move(rec));

            item->resetMove();
        }

        if (item->isResizeStarted())
        {
            LayoutResizeCommand::Record rec;
            rec.path = item->getWidgetPath();
            rec.oldPos = item->getStartPosition();
            rec.newPos = item->getWidget()->getPosition();
            rec.oldSize = item->getStartSize();
            rec.newSize = item->getWidget()->getSize();
            resize.push_back(std::move(rec));

            item->resetResize();
        }
    }

    if (!move.empty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(move)));

    if (!resize.empty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(resize)));
}
