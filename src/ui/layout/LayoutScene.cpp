#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/AnchorCornerHandle.h"
#include "src/ui/layout/AnchorEdgeHandle.h"
#include "src/ui/layout/AnchorPopupMenu.h"
#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/NumericValueItem.h"
#include "src/ui/ResizingHandle.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIManager.h" //!!!for OpenGL context! TODO: encapsulate?
#include <CEGUI/CoordConverter.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/Window.h>
#include "qgraphicssceneevent.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qtreeview.h"
#include "qstandarditemmodel.h"
#include <qmenu.h>
#include <set>

// For properties (may be incapsulated somewhere):
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include <QtnProperty/PropertyWidget.h>
#include <QtnProperty/PropertySet.h>
#include <QtnProperty/PropertyView.h>
#include <QtnProperty/MultiProperty.h>

LayoutScene::LayoutScene(LayoutVisualMode& visualMode)
    : CEGUIGraphicsScene(&visualMode)
    , _visualMode(visualMode)
{
    connect(this, &LayoutScene::selectionChanged, this, &LayoutScene::onSelectionChanged);
}

LayoutScene::~LayoutScene()
{
    if (_multiSet) _multiSet->clearChildProperties();
    disconnect(this, &LayoutScene::selectionChanged, this, &LayoutScene::onSelectionChanged);
    delete _anchorPopupMenu;
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

    if (_multiSet) _multiSet->clearChildProperties();

    // Clear scene without reacting on selection changes. Will update once at the end when items recreated.
    disconnect(this, &LayoutScene::selectionChanged, this, &LayoutScene::onSelectionChanged);
    clear();
    connect(this, &LayoutScene::selectionChanged, this, &LayoutScene::onSelectionChanged);

    rootManipulator = manipulator;

    if (rootManipulator)
    {
        CEGUIManager::Instance().makeOpenGLContextCurrent();
        ceguiContext->setRootWindow(rootManipulator->getWidget());
        CEGUIManager::Instance().doneOpenGLContextCurrent();

        // Root manipulator changed, perform a full update
        // NB: widget must be already set to a CEGUI context for area calculation
        rootManipulator->updateFromWidget(true);
        addItem(rootManipulator);

        createAnchorItems();

        Application* app = qobject_cast<Application*>(qApp);
        rootManipulator->showLayoutContainerHandles(app->getAction("layout/show_lc_handles")->isChecked());
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

    // Finally update the selection to reflect that there is nothing selected
    emit selectionChanged();
}

LayoutManipulator* LayoutScene::getManipulatorByPath(const QString& widgetPath) const
{
    if (!rootManipulator || widgetPath.isEmpty()) return nullptr;

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

bool LayoutScene::deleteWidgetByPath(const QString& widgetPath)
{
    LayoutManipulator* manipulator = getManipulatorByPath(widgetPath);
    if (!manipulator) return false;

    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    auto propertyWidget = static_cast<QtnPropertyWidget*>(mainWindow->getPropertyDockWidget()->widget());
    if (propertyWidget->propertySet() == manipulator->getPropertySet())
        propertyWidget->setPropertySet(nullptr);
    if (_multiSet)
        _multiSet->clearChildProperties();

    auto parentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem());

    manipulator->detach();
    delete manipulator;

    // Mostly for the LC case, its area depends on the children
    if (parentManipulator)
        parentManipulator->updateFromWidget(true, true);

    return true;
}

size_t LayoutScene::getMultiSelectionChangeId() const
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    auto propertyWidget = static_cast<QtnPropertyWidget*>(mainWindow->getPropertyDockWidget()->widget());
    return (_multiSet && propertyWidget->propertySet() == _multiSet) ? _multiChangeId : 0;
}

void LayoutScene::updatePropertySet()
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);
    updatePropertySet(selectedWidgets);
}

void LayoutScene::updatePropertySet(const std::set<LayoutManipulator*>& selectedWidgets)
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    auto propertyDockWidget = mainWindow->getPropertyDockWidget();
    auto propertyWidget = static_cast<QtnPropertyWidget*>(propertyDockWidget->widget());

    disconnect(propertyWidget->propertyView(), &QtnPropertyView::beforePropertyEdited, this, &LayoutScene::onBeforePropertyEdited);

    if (selectedWidgets.size() == 1)
    {
        auto selectedWidget = *selectedWidgets.begin();
        propertyWidget->setPropertySet(selectedWidget->getPropertySet());
        propertyDockWidget->setWindowTitle("Properties: " + selectedWidget->getWidgetName());
        // TODO: selectedWidget->getWidgetPath() in the header tooltip (not for the whole propertyWidget tooltip!)
    }
    else if (selectedWidgets.size() > 1)
    {
        connect(propertyWidget->propertyView(), &QtnPropertyView::beforePropertyEdited, this, &LayoutScene::onBeforePropertyEdited);

        if (_multiSet)
        {
            // Unset our multiset from the widget to avoid freeze due to contents change
            propertyWidget->setPropertySet(nullptr);
            _multiSet->clearChildProperties();
        }
        else
            _multiSet = new QtnPropertySet(this);

        for (LayoutManipulator* manipulator : selectedWidgets)
            qtnPropertiesToMultiSet(_multiSet, manipulator->getPropertySet(), false);

        propertyWidget->setPropertySet(_multiSet);
        propertyDockWidget->setWindowTitle(QString("Properties: %1 widgets").arg(selectedWidgets.size()));
    }
    else
    {
        propertyWidget->setPropertySet(nullptr);
        propertyDockWidget->setWindowTitle("Properties");
    }
}

void LayoutScene::normalizePositionOfSelectedWidgets()
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutMoveCommand::Record> records;
    bool toRelative = false;

    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        const auto pos = manipulator->getWidget()->getPosition();

        LayoutMoveCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = pos;
        records.push_back(std::move(rec));

        // For absolute-relative cycling
        if (!toRelative && (pos.d_x.d_offset != 0.f || pos.d_y.d_offset != 0.f))
            toRelative = true;
    }

    for (auto& rec : records)
    {
        const LayoutManipulator* manipulator = getManipulatorByPath(rec.path);
        const auto baseSize = manipulator->getBaseSize();

        if (toRelative)
        {
            rec.newPos.d_x.d_scale = (rec.oldPos.d_x.d_offset + rec.oldPos.d_x.d_scale * baseSize.d_width) / baseSize.d_width;
            rec.newPos.d_x.d_offset = 0.f;
            rec.newPos.d_y.d_scale = (rec.oldPos.d_y.d_offset + rec.oldPos.d_y.d_scale * baseSize.d_height) / baseSize.d_height;
            rec.newPos.d_y.d_offset = 0.f;
        }
        else
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
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutResizeCommand::Record> records;
    bool toRelative = false;

    for (LayoutManipulator* manipulator : selectedWidgets)
    {
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
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutMoveCommand::Record> records;
    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        LayoutMoveCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = manipulator->getWidget()->getPosition();
        rec.newPos = rec.oldPos;
        rec.newPos.d_x.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_x.d_offset);
        rec.newPos.d_y.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_y.d_offset);
        records.push_back(std::move(rec));
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
}

void LayoutScene::roundSizeOfSelectedWidgets()
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutResizeCommand::Record> records;
    for (LayoutManipulator* manipulator : selectedWidgets)
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

    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(records)));
}

void LayoutScene::alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment)
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutHorizontalAlignCommand::Record> records;
    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        LayoutHorizontalAlignCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldAlignment = manipulator->getWidget()->getHorizontalAlignment();
        records.push_back(std::move(rec));
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutHorizontalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::alignSelectionVertically(CEGUI::VerticalAlignment alignment)
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    std::vector<LayoutVerticalAlignCommand::Record> records;
    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        LayoutVerticalAlignCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldAlignment = manipulator->getWidget()->getVerticalAlignment();
        records.push_back(std::move(rec));
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutVerticalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::moveSelectedWidgetsInParentWidgetLists(int delta)
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    QStringList paths;
    for (LayoutManipulator* manipulator : selectedWidgets)
    {
        auto parentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem());
        if (!parentManipulator) continue;

        auto parent = parentManipulator->getWidget();

        const int potentialPos = static_cast<int>(parent->getChildIndex(manipulator->getWidget())) + delta;
        if (potentialPos < 0 || potentialPos >= static_cast<int>(parent->getChildCount())) continue;

        paths.append(manipulator->getWidgetPath());
    }

    if (paths.empty()) return;

    _visualMode.getEditor().getUndoStack()->push(new MoveInParentWidgetListCommand(_visualMode, std::move(paths), delta));
}

bool LayoutScene::deleteSelectedWidgets()
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return true;

    QStringList widgetPaths;
    for (LayoutManipulator* manipulator : selectedWidgets)
        widgetPaths.push_back(manipulator->getWidgetPath());

    _visualMode.getEditor().getUndoStack()->push(new LayoutDeleteCommand(_visualMode, std::move(widgetPaths)));

    return true;
}

void LayoutScene::showLayoutContainerHandles(bool show)
{
    if (rootManipulator) rootManipulator->showLayoutContainerHandles(show);
}

void LayoutScene::selectParent()
{
    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    if (selectedWidgets.empty()) return;

    auto manipulator = *selectedWidgets.begin();
    if (auto parentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem()))
    {
        clearSelection();
        parentManipulator->setSelected(true);
    }
}

void LayoutScene::onManipulatorRemoved(LayoutManipulator* manipulator)
{
    if (_anchorTarget == manipulator) _anchorTarget = nullptr;
}

void LayoutScene::onManipulatorUpdatedFromWidget(LayoutManipulator* manipulator)
{
    if (!manipulator) return;

    // Update anchor handles for the manipulator if they are shown,
    // unless updateFromWidget() was called due to dragging them
    if (_anchorTarget == manipulator)
    {
        const bool isAnchorHandleSelected = (getCurrentAnchorItem() != nullptr);
        if (!_anchorTarget->resizeInProgress() || !isAnchorHandleSelected)
        {
            updateAnchorItems();
            updateAnchorValueItems();
        }

        if (!isAnchorHandleSelected)
        {
            _anchorTextX->setVisible(false);
            _anchorTextY->setVisible(false);
        }
    }
}

LayoutManipulator* LayoutScene::getManipulatorFromItem(QGraphicsItem* item) const
{
    if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
    {
        return manipulator;
    }
    else if (dynamic_cast<ResizingHandle*>(item) || dynamic_cast<LayoutContainerHandle*>(item))
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item->parentItem()))
            return manipulator;
    }

    return nullptr;
}

void LayoutScene::collectSelectedWidgets(std::set<LayoutManipulator*>& selectedWidgets)
{
    auto selection = selectedItems();
    for (QGraphicsItem* item : selection)
    {
        if (auto manipulator = getManipulatorFromItem(item))
            selectedWidgets.insert(manipulator);
    }
}

static void ensureParentIsExpanded(QTreeView* view, QStandardItem* treeItem)
{
    view->expand(treeItem->index());
    if (treeItem->parent())
        ensureParentIsExpanded(view, treeItem->parent());
}

void LayoutScene::onSelectionChanged()
{
    if (_batchSelection) return;

    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    // Only one selected anchor item at a time is allowed
    QGraphicsItem* selectedAnchorItem = getCurrentAnchorItem();

    // Update anchors state

    if (selectedWidgets.size() > 1)
    {
        // Hide anchors if we selected multiple widgets
        _anchorTarget = nullptr;
    }
    else if (selectedWidgets.size() == 0)
    {
        if (selectedAnchorItem)
        {
            // If we selected anchor item, we therefore deselected an _anchorTarget,
            // but it must look as selected in a GUI, so we don't change anything
            selectedWidgets.insert(_anchorTarget);
        }
        else
        {
            // Nothing interesting is selected, hide anchors
            _anchorTarget = nullptr;
        }
    }
    else
    {
        // Show anchors for the only selected widget
        _anchorTarget = *selectedWidgets.begin();
    }

    if (_anchorTextX) _anchorTextX->setVisible(false);
    if (_anchorTextY) _anchorTextY->setVisible(false);

    updateAnchorItems();

    // Update property view for our selection

    updatePropertySet(selectedWidgets);

    // Show selection in a hierarchy tree

    if (!_ignoreSelectionChanges)
    {
        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(true);

        auto treeView = _visualMode.getHierarchyDockWidget()->getTreeView();
        treeView->clearSelection();

        QStandardItem* lastTreeItem = nullptr;
        for (LayoutManipulator* manipulator : selectedWidgets)
        {
            if (auto treeItem = manipulator->getTreeItem())
            {
                treeView->selectionModel()->select(treeItem->index(), QItemSelectionModel::Select);
                ensureParentIsExpanded(treeView, treeItem);
                lastTreeItem = treeItem;
            }
        }

        if (lastTreeItem) treeView->scrollTo(lastTreeItem->index());

        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(false);
    }
}

void LayoutScene::onBeforePropertyEdited()
{
    // We count user property editing events and group all property changes from
    // a single event to one undo command for multiproperties
    ++_multiChangeId;
}

void LayoutScene::createAnchorItems()
{
    QPen anchorRectPen(Qt::PenStyle::SolidLine);
    anchorRectPen.setColor(Qt::magenta);
    anchorRectPen.setWidth(3);
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

bool LayoutScene::isAnchorItem(QGraphicsItem* item) const
{
    if (!item) return false;

    return item == _anchorMinX ||
            item == _anchorMinY ||
            item == _anchorMaxX ||
            item == _anchorMaxY ||
            item == _anchorMinXMinY ||
            item == _anchorMaxXMinY ||
            item == _anchorMinXMaxY ||
            item == _anchorMaxXMaxY;
}

QGraphicsItem* LayoutScene::getCurrentAnchorItem() const
{
    // Too early call, items aren't created yet
    if (!_anchorMinX) return nullptr;

    if (_anchorMinX->isSelected()) return _anchorMinX;
    if (_anchorMaxX->isSelected()) return _anchorMaxX;
    if (_anchorMinY->isSelected()) return _anchorMinY;
    if (_anchorMaxY->isSelected()) return _anchorMaxY;
    if (_anchorMinXMinY->isSelected()) return _anchorMinXMinY;
    if (_anchorMaxXMinY->isSelected()) return _anchorMaxXMinY;
    if (_anchorMinXMaxY->isSelected()) return _anchorMinXMaxY;
    if (_anchorMaxXMaxY->isSelected()) return _anchorMaxXMaxY;
    return nullptr;
}

void LayoutScene::deselectAllAnchorItems()
{
    // Too early call, items aren't created yet
    if (!_anchorMinX) return;

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
}

void LayoutScene::updateAnchorItems(QGraphicsItem* movedItem)
{
    // Too early call, items aren't created yet
    if (!_anchorParentRect) return;

    Application* app = qobject_cast<Application*>(qApp);
    bool showAnchors = (_anchorTarget != nullptr && app->getAction("layout/show_anchors")->isChecked());

    const bool showPosAnchors = showAnchors && (!_anchorTarget->isInLayoutContainer());
    const bool showSizeAnchors = showAnchors && !_anchorTarget->isLayoutContainer();

    // If no particular anchors are shown, we effectively don't show anchors at all
    if (!showPosAnchors && !showSizeAnchors)
        showAnchors = false;

    _anchorParentRect->setVisible(showAnchors);
    _anchorMinX->setVisible(showPosAnchors);
    _anchorMinY->setVisible(showPosAnchors);
    _anchorMaxX->setVisible(showSizeAnchors);
    _anchorMaxY->setVisible(showSizeAnchors);
    _anchorMinXMinY->setVisible(showPosAnchors);
    _anchorMaxXMinY->setVisible(showPosAnchors && showSizeAnchors);
    _anchorMinXMaxY->setVisible(showPosAnchors && showSizeAnchors);
    _anchorMaxXMaxY->setVisible(showSizeAnchors);

    // Stop the current operation if the dragged item became invisible.
    // NB: movedItem may be nullptr here but dragging can still occur.
    const bool stop = !showAnchors || (movedItem && !movedItem->isVisible());
    if (stop && _anchorTarget && _anchorTarget->resizeInProgress())
        _anchorTarget->endResizing();

    if (showAnchors)
    {
        _anchorParentRect->setRect(_anchorTarget->getParentSceneRect().adjusted(-1.0, -1.0, 1.0, 1.0));

        // Set handle positions without firing move events
        const QRectF anchorsRect = _anchorTarget->getAnchorsSceneRect();
        if (showPosAnchors)
        {
            if (movedItem != _anchorMinX) _anchorMinX->setPosSilent(anchorsRect.left(), 0.0);
            if (movedItem != _anchorMinY) _anchorMinY->setPosSilent(0.0, anchorsRect.top());
            if (movedItem != _anchorMinXMinY) _anchorMinXMinY->setPosSilent(anchorsRect.left(), anchorsRect.top());
        }
        if (showSizeAnchors)
        {
            if (movedItem != _anchorMaxX) _anchorMaxX->setPosSilent(anchorsRect.right(), 0.0);
            if (movedItem != _anchorMaxY) _anchorMaxY->setPosSilent(0.0, anchorsRect.bottom());
            if (movedItem != _anchorMaxXMaxY) _anchorMaxXMaxY->setPosSilent(anchorsRect.right(), anchorsRect.bottom());
        }
        if (showPosAnchors && showSizeAnchors)
        {
            if (movedItem != _anchorMaxXMinY) _anchorMaxXMinY->setPosSilent(anchorsRect.right(), anchorsRect.top());
            if (movedItem != _anchorMinXMaxY) _anchorMinXMaxY->setPosSilent(anchorsRect.left(), anchorsRect.bottom());
        }
    }
    else
    {
        _anchorTextX->setVisible(false);
        _anchorTextY->setVisible(false);
        deselectAllAnchorItems();
    }
}

// Updates position and value of anchor texts corresponding to the anchor item passed
void LayoutScene::updateAnchorValueItems()
{
    if (!_anchorTarget) return;

    const bool hasMinX = _anchorMinX->isSelected() || _anchorMinXMinY->isSelected() || _anchorMinXMaxY->isSelected();
    const bool hasMaxX = _anchorMaxX->isSelected() || _anchorMaxXMinY->isSelected() || _anchorMaxXMaxY->isSelected();
    const bool hasMinY = _anchorMinY->isSelected() || _anchorMinXMinY->isSelected() || _anchorMaxXMinY->isSelected();
    const bool hasMaxY = _anchorMaxY->isSelected() || _anchorMinXMaxY->isSelected() || _anchorMaxXMaxY->isSelected();

    const auto widgetCenter = _anchorTarget->sceneBoundingRect().center();

    if (hasMinX)
    {
        _anchorTextX->setValue(static_cast<qreal>(_anchorTarget->getAnchorMinX()) * 100.0);
        _anchorTextX->setX(_anchorMinX->sceneBoundingRect().left() - _anchorTextX->sceneBoundingRect().width());
        _anchorTextX->setY(widgetCenter.y() - _anchorTextX->sceneBoundingRect().height() / 2.0);
    }
    else if (hasMaxX)
    {
        _anchorTextX->setValue(static_cast<qreal>(_anchorTarget->getAnchorMaxX()) * 100.0);
        _anchorTextX->setX(_anchorMaxX->sceneBoundingRect().right());
        _anchorTextX->setY(widgetCenter.y() - _anchorTextX->sceneBoundingRect().height() / 2.0);
    }

    if (hasMinY)
    {
        _anchorTextY->setValue(static_cast<qreal>(_anchorTarget->getAnchorMinY()) * 100.0);
        _anchorTextY->setX(widgetCenter.x() - _anchorTextY->sceneBoundingRect().width() / 2.0);
        _anchorTextY->setY(_anchorMinY->sceneBoundingRect().top() - _anchorTextY->sceneBoundingRect().height());
    }
    else if (hasMaxY)
    {
        _anchorTextY->setValue(static_cast<qreal>(_anchorTarget->getAnchorMaxY()) * 100.0);
        _anchorTextY->setX(widgetCenter.x() - _anchorTextY->sceneBoundingRect().width() / 2.0);
        _anchorTextY->setY(_anchorMaxY->sceneBoundingRect().bottom());
    }
}

bool LayoutScene::getAnchorValues(float& minX, float& maxX, float& minY, float& maxY) const
{
    if (!_anchorTarget) return false;
    minX = _anchorTarget->getAnchorMinX();
    maxX = _anchorTarget->getAnchorMaxX();
    minY = _anchorTarget->getAnchorMinY();
    maxY = _anchorTarget->getAnchorMaxY();
    return true;
}

void LayoutScene::setAnchorValues(float minX, float maxX, float minY, float maxY, bool preserveEffectiveSize)
{
    if (!_anchorTarget) return;

    LayoutResizeCommand::Record rec;
    rec.path = _anchorTarget->getWidgetPath();
    rec.oldPos = _anchorTarget->getWidget()->getPosition();
    rec.oldSize = _anchorTarget->getWidget()->getSize();

    _anchorTarget->setAnchors(minX, maxX, minY, maxY, preserveEffectiveSize);

    rec.newPos = _anchorTarget->getWidget()->getPosition();
    rec.newSize = _anchorTarget->getWidget()->getSize();

    std::vector<LayoutResizeCommand::Record> resize;
    resize.push_back(std::move(rec));
    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(resize)));

    updateAnchorItems();
    updateAnchorValueItems();
}

// TODO: snap to the parent rect? as setting?
// FIXME: when snapping to edge of self with Shift, handle dragging is not smooth
void LayoutScene::anchorHandleMoved(QGraphicsItem* item, QPointF& newPos, bool moveOpposite)
{
    float minX, maxX, minY, maxY;
    if (!item || !getAnchorValues(minX, maxX, minY, maxY)) return;

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
            const auto siblingAnchorsRect = siblingManipulator->getAnchorsSceneRect();
            auto anchorPos = newPos;

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

                newPos = anchorPos;
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

                newPos = anchorPos;
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

    // Do constraining and dependent moving

    const QPointF newAnchor = _anchorTarget->scenePixelToAnchor(newPos);

    if (item == _anchorMinX || item == _anchorMinXMinY || item == _anchorMinXMaxY)
    {
        minX = static_cast<float>(newAnchor.x());
        if (maxX < minX)
        {
            if (moveOpposite || !_anchorMaxX->isVisible()) maxX = minX;
            else
            {
                minX = maxX;
                newPos.setX(_anchorMaxX->pos().x());
            }
        }
    }
    else if (item == _anchorMaxX || item == _anchorMaxXMinY || item == _anchorMaxXMaxY)
    {
        maxX = static_cast<float>(newAnchor.x());
        if (maxX < minX)
        {
            if (moveOpposite || !_anchorMinX->isVisible()) minX = maxX;
            else
            {
                maxX = minX;
                newPos.setX(_anchorMinX->pos().x());
            }
        }
    }

    if (item == _anchorMinY || item == _anchorMinXMinY || item == _anchorMaxXMinY)
    {
        minY = static_cast<float>(newAnchor.y());
        if (maxY < minY)
        {
            if (moveOpposite || !_anchorMaxY->isVisible()) maxY = minY;
            else
            {
                minY = maxY;
                newPos.setY(_anchorMaxY->pos().y());
            }
        }
    }
    else if (item == _anchorMaxY || item == _anchorMinXMaxY || item == _anchorMaxXMaxY)
    {
        maxY = static_cast<float>(newAnchor.y());
        if (maxY < minY)
        {
            if (moveOpposite || !_anchorMinY->isVisible()) minY = maxY;
            else
            {
                maxY = minY;
                newPos.setY(_anchorMinY->pos().y());
            }
        }
    }

    // Perform actual changes. We don't use setAnchorValues() because we don't want
    // to create undo commands for each change. They are created once at mouse up.

    // When draggind with Ctrl pressed, target is reselected for some internal Qt reason.
    // Deselecting it prevents its moving, which otherwise would break resizing.
    if (_anchorTarget->isSelected())
    {
        item->setSelected(true);
        _anchorTarget->setSelected(false);
    }

    if (!_anchorTarget->resizeInProgress())
        _anchorTarget->beginResizing(*item);

    const bool preserveEffectiveSize = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
    _anchorTarget->setAnchors(minX, maxX, minY, maxY, preserveEffectiveSize);

    updateAnchorItems(item);
    updateAnchorValueItems();
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

    // If anchor handle is selected, target must not be selected
    // to avoid simultaneous move problems
    if (item && _anchorTarget->isSelected())
        _anchorTarget->setSelected(false);

    // TODO: preserveEffectiveSize - need hotkey or option!

    if (item == _anchorMinX || item == _anchorMinXMinY || item == _anchorMinXMaxY)
    {
        _anchorTextX->setVisible(true);
        _anchorTextX->setHorizontalAlignment(Qt::AlignRight);
        _anchorTextX->setTextTemplate("%1%");
        connect(_anchorTextX, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minX = static_cast<float>(newValue) / 100.f;
            const float maxX = std::max(_anchorTarget->getAnchorMaxX(), minX);
            setAnchorValues(minX, maxX, _anchorTarget->getAnchorMinY(), _anchorTarget->getAnchorMaxY(), false);
        });
     }
    else if (item == _anchorMaxX || item == _anchorMaxXMinY || item == _anchorMaxXMaxY)
    {
        _anchorTextX->setVisible(true);
        _anchorTextX->setHorizontalAlignment(Qt::AlignLeft);
        _anchorTextX->setTextTemplate("%1%");
        connect(_anchorTextX, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float maxX = static_cast<float>(newValue) / 100.f;
            const float minX = std::min(_anchorTarget->getAnchorMinX(), maxX);
            setAnchorValues(minX, maxX, _anchorTarget->getAnchorMinY(), _anchorTarget->getAnchorMaxY(), false);
        });
    }
    else _anchorTextX->setVisible(false);

    if (item == _anchorMinY || item == _anchorMinXMinY || item == _anchorMaxXMinY)
    {
        _anchorTextY->setVisible(true);
        _anchorTextY->setTextTemplate("%1%");
        connect(_anchorTextY, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float minY = static_cast<float>(newValue) / 100.f;
            const float maxY = std::max(_anchorTarget->getAnchorMaxY(), minY);
            setAnchorValues(_anchorTarget->getAnchorMinX(), _anchorTarget->getAnchorMaxX(), minY, maxY, false);
        });
    }
    else if (item == _anchorMaxY || item == _anchorMinXMaxY || item == _anchorMaxXMaxY)
    {
        _anchorTextY->setVisible(true);
        _anchorTextY->setTextTemplate("%1%");
        connect(_anchorTextY, &NumericValueItem::valueChanged, [this](qreal newValue)
        {
            const float maxY = static_cast<float>(newValue) / 100.f;
            const float minY = std::min(_anchorTarget->getAnchorMinY(), maxY);
            setAnchorValues(_anchorTarget->getAnchorMinX(), _anchorTarget->getAnchorMaxX(), minY, maxY, false);
        });
    }
    else _anchorTextY->setVisible(false);

    updateAnchorValueItems();
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
        // Dropping to an empty scene. Dropped widget becomes a root.
        auto data = event->mimeData()->data("application/x-ceed-widget-type");
        if (data.size() > 0)
        {
            QString widgetType = data.data();
            QString widgetName;
            if (widgetType == "DefaultWindow")
            {
                // Special case - root widget. Setup it with most useful parameters.
                // Naming convention is from docs:
                // http://static.cegui.org.uk/docs/0.8.7/window_tutorial.html
                widgetName = "root";
                // can instead use: widgetName = QFileInfo(_visualMode.getEditor().getFilePath()).baseName();
            }
            else
            {
                const int sepPos = widgetType.lastIndexOf('/');
                widgetName = (sepPos < 0) ? widgetType : widgetType.mid(sepPos + 1);
            }

            _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, "", widgetType, widgetName, event->scenePos()));

            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
        }
    }
}

void LayoutScene::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            std::set<LayoutManipulator*> selectedWidgets;
            collectSelectedWidgets(selectedWidgets);

            if (!selectedWidgets.empty())
            {
                QPointF delta(0.0, 0.0);
                switch (event->key())
                {
                    case Qt::Key_Left: delta.setX(-1.0); break;
                    case Qt::Key_Right: delta.setX(1.0); break;
                    case Qt::Key_Up: delta.setY(-1.0); break;
                    case Qt::Key_Down: delta.setY(1.0); break;
                }

                if (delta.manhattanLength() > 0.0)
                {
                    if (event->modifiers() & Qt::ControlModifier) delta *= 10.0;

                    if (event->modifiers() & Qt::ShiftModifier)
                    {
                        const CEGUI::USize ceguiDelta(
                                    CEGUI::UDim(0.0, static_cast<float>(delta.x())),
                                    CEGUI::UDim(0.0, static_cast<float>(delta.y())));
                        std::vector<LayoutResizeCommand::Record> resize;
                        for (LayoutManipulator* item : selectedWidgets)
                        {
                            item->resetMove();
                            item->resetResize();
                            LayoutResizeCommand::Record rec;
                            rec.path = item->getWidgetPath();
                            rec.oldPos = item->getWidget()->getPosition();
                            rec.newPos = item->getWidget()->getPosition();
                            rec.oldSize = item->getWidget()->getSize();
                            rec.newSize = item->getWidget()->getSize() + ceguiDelta;
                            resize.push_back(std::move(rec));
                        }
                        _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(resize)));
                    }
                    else
                    {
                        const CEGUI::UVector2 ceguiDelta(
                                    CEGUI::UDim(0.0, static_cast<float>(delta.x())),
                                    CEGUI::UDim(0.0, static_cast<float>(delta.y())));
                        std::vector<LayoutMoveCommand::Record> move;
                        for (LayoutManipulator* item : selectedWidgets)
                        {
                            item->resetMove();
                            item->resetResize();
                            LayoutMoveCommand::Record rec;
                            rec.path = item->getWidgetPath();
                            rec.oldPos = item->getWidget()->getPosition();
                            rec.newPos = item->getWidget()->getPosition() + ceguiDelta;
                            move.push_back(std::move(rec));
                        }
                        _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(move)));
                    }

                    event->accept();
                    return;
                }
            }
        }
    }

    CEGUIGraphicsScene::keyPressEvent(event);
}

void LayoutScene::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    if (event->key() == Qt::Key_Delete)
    {
        handled = deleteSelectedWidgets();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        if (!selectedItems().isEmpty())
        {
            clearSelection();
            handled = true;
        }
    }
    else if (event->matches(QKeySequence::SelectAll))
    {
        // UX: select only siblings of already selected, if any?
        batchSelection(true);
        clearSelection();
        for (auto item : items())
            if (dynamic_cast<LayoutManipulator*>(item))
                item->setSelected(true);
        batchSelection(false);
        emit selectionChanged();
        handled = true;
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

    std::set<LayoutManipulator*> selectedWidgets;
    collectSelectedWidgets(selectedWidgets);

    // Detect resizing by anchors not through selected items but through
    // the state of the anchored item itself
    if (_anchorTarget && _anchorTarget->resizeInProgress())
    {
        _anchorTarget->endResizing();
        selectedWidgets.insert(_anchorTarget);
    }

    std::vector<LayoutMoveCommand::Record> move;
    std::vector<LayoutResizeCommand::Record> resize;

    for (LayoutManipulator* item : selectedWidgets)
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

void LayoutScene::showAnchorPopupMenu(const QPoint& pos)
{
    if (!_anchorPopupMenu)
        _anchorPopupMenu = new AnchorPopupMenu(*this);
    _anchorPopupMenu->move(pos);
    _anchorPopupMenu->show();
}

void LayoutScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Get all items at mouse position and find a topmost interesting
    const auto itemsAtMouse = items(event->scenePos());
    for (QGraphicsItem* itemAtMouse : itemsAtMouse)
    {
        if (_anchorTarget && isAnchorItem(itemAtMouse))
        {
            showAnchorPopupMenu(event->scenePos().toPoint());
            event->accept();
            return;
        }

        if (auto manipulator = getManipulatorFromItem(itemAtMouse))
        {
            // If target manipulator is not in a current selection, select it
            if (!manipulator->isSelected() && !manipulator->isAnyHandleSelected())
            {
                clearSelection();
                manipulator->setSelected(true);
            }

            // Let's show a context menu for this manipulator
            break;
        }
    }

    // Menu is shown for the current selection
    auto contextMenu = _visualMode.getContextMenu();
    contextMenu->move(event->screenPos());
    contextMenu->show();

    event->accept();
}
