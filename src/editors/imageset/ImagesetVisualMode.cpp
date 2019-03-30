#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/ConfigurableAction.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImageOffsetMark.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/ui/ResizingHandle.h"
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include "qopenglwidget.h"
#include "qclipboard.h"
#include "qmimedata.h"
#include "qtoolbar.h"
#include "qevent.h"
#include "qmenu.h"
#include "qdom.h"

constexpr qreal newImageHalfSize = 25.0;

ImagesetVisualMode::ImagesetVisualMode(MultiModeEditor& editor)
    : IEditMode(editor)
    , lastCursorPosition(newImageHalfSize, newImageHalfSize)
{
    wheelZoomEnabled = true;
    middleButtonDragScrollEnabled = true;

    setScene(new QGraphicsScene());

    setFocusPolicy(Qt::ClickFocus);
    setFrameStyle(QFrame::NoFrame);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    if (settings->getEntryValue("imageset/visual/partial_updates").toBool())
    {
        // the commented lines are possible optimisation, I found out that they don't really
        // speed it up in a noticeable way so I commented them out

        //setOptimizationFlag(DontSavePainterState, true);
        //setOptimizationFlag(DontAdjustForAntialiasing, true);
        //setCacheMode(CacheBackground);
        setViewportUpdateMode(MinimalViewportUpdate);
        //setRenderHint(QPainter::Antialiasing, false);
        //setRenderHint(QPainter::TextAntialiasing, false);
        //setRenderHint(QPainter::SmoothPixmapTransform, false);
    }
    else
    {
        // use OpenGL for view redrawing
        // depending on the platform and hardware this may be faster or slower
        setViewport(new QOpenGLWidget());
        setViewportUpdateMode(FullViewportUpdate);
    }

    setDragMode(RubberBandDrag);
    setBackgroundBrush(QBrush(Qt::lightGray));

    connect(scene(), &QGraphicsScene::selectionChanged, this, &ImagesetVisualMode::slot_selectionChanged);

    dockWidget = new ImagesetEditorDockWidget(*this);

    setupActions();
}

void ImagesetVisualMode::setupActions()
{
    Application* app = qobject_cast<Application*>(qApp);

    auto&& settings = app->getSettings();
    auto category = settings->getCategory("shortcuts");
    if (!category) category = settings->createCategory("shortcuts", "Shortcuts");
    auto section = category->createSection("imageset", "Imageset Editor");

    auto mainWindow = app->getMainWindow();

    editOffsetsAction = new ConfigurableAction(mainWindow,
                                               *section, "edit_offsets", "Edit &Offsets",
                                               "When you select an image definition, a crosshair will appear in it representing it's offset centrepoint.",
                                               QIcon(":/icons/imageset_editing/edit_offsets.png"), QKeySequence(Qt::Key_Space));
    editOffsetsAction->setCheckable(true);
    connect(editOffsetsAction, &ConfigurableAction::toggled, this, &ImagesetVisualMode::slot_toggleEditOffsets);

    cycleOverlappingAction = new ConfigurableAction(mainWindow,
                                               *section, "cycle_overlapping", "Cycle O&verlapping Image Definitions",
                                               "When images definition overlap in such a way that makes it hard/impossible to select the definition you want, this allows you to select on of them and then just cycle until the right one is selected.",
                                               QIcon(":/icons/imageset_editing/cycle_overlapping.png"), QKeySequence(Qt::Key_Q));
    connect(cycleOverlappingAction, &ConfigurableAction::triggered, this, &ImagesetVisualMode::cycleOverlappingImages);

    createImageAction = new ConfigurableAction(mainWindow,
                                               *section, "create_image", "&Create Image Definition",
                                               "Creates a new image definition at the current cursor position, sized 50x50 pixels.",
                                               QIcon(":/icons/imageset_editing/create_image.png"));
    connect(createImageAction, &ConfigurableAction::triggered, this, &ImagesetVisualMode::createImageEntryAtCursor);

    duplicateSelectedImagesAction = new ConfigurableAction(mainWindow,
                                               *section, "duplicate_image", "&Duplicate Image Definition",
                                               "Duplicates selected image definitions.",
                                               QIcon(":/icons/imageset_editing/duplicate_image.png"));
    connect(duplicateSelectedImagesAction, &ConfigurableAction::triggered, this, &ImagesetVisualMode::duplicateSelectedImageEntries);

    focusImageListFilterBoxAction = new ConfigurableAction(mainWindow,
                                               *section, "focus_image_list_filter_box", "&Filter...",
                                               "This allows you to easily press a shortcut and immediately search through image definitions without having to reach for a mouse.",
                                               QIcon(":/icons/imageset_editing/focus_image_list_filter_box.png"),
                                               QKeySequence(QKeySequence::Find));
    connect(focusImageListFilterBoxAction, &ConfigurableAction::triggered, [this]()
    {
        dockWidget->focusImageListFilterBox();
    });

    setActionsEnabled(false);

    toolBar = mainWindow->createToolbar("Imageset");
    toolBar->addAction(createImageAction);
    toolBar->addAction(duplicateSelectedImagesAction);
    toolBar->addSeparator();
    toolBar->addAction(editOffsetsAction);
    toolBar->addAction(cycleOverlappingAction);

    contextMenu = new QMenu(this);
    contextMenu->addAction(createImageAction);
    contextMenu->addAction(duplicateSelectedImagesAction);
    contextMenu->addAction(mainWindow->getActionDeleteSelected());
    contextMenu->addSeparator();
    contextMenu->addAction(cycleOverlappingAction);
    contextMenu->addSeparator();
    contextMenu->addAction(mainWindow->getActionZoomIn());
    contextMenu->addAction(mainWindow->getActionZoomOut());
    contextMenu->addAction(mainWindow->getActionZoomReset());
    contextMenu->addSeparator();
    contextMenu->addAction(editOffsetsAction);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &ImagesetVisualMode::customContextMenuRequested, this, &ImagesetVisualMode::slot_customContextMenu);
}

void ImagesetVisualMode::loadImagesetEntryFromElement(const QDomElement& xmlRoot)
{
    scene()->clear();

    imagesetEntry = new ImagesetEntry(*this);
    imagesetEntry->loadFromElement(xmlRoot);
    scene()->addItem(imagesetEntry);

    refreshSceneRect();

    dockWidget->setImagesetEntry(imagesetEntry);
    dockWidget->refresh();
}

void ImagesetVisualMode::rebuildEditorMenu(QMenu* editorMenu)
{
    // Similar to the toolbar, includes the focus filter box action
    editorMenu->addAction(createImageAction);
    editorMenu->addAction(duplicateSelectedImagesAction);
    editorMenu->addSeparator();
    editorMenu->addAction(cycleOverlappingAction);
    editorMenu->addSeparator();
    editorMenu->addAction(editOffsetsAction);
    editorMenu->addSeparator();
    editorMenu->addAction(focusImageListFilterBoxAction);
    _editorMenu = editorMenu;
}

void ImagesetVisualMode::setActionsEnabled(bool enabled)
{
    editOffsetsAction->setEnabled(enabled);
    createImageAction->setEnabled(enabled);
    duplicateSelectedImagesAction->setEnabled(enabled);
    cycleOverlappingAction->setEnabled(enabled);
    focusImageListFilterBoxAction->setEnabled(enabled);
}

void ImagesetVisualMode::refreshSceneRect()
{
    // The reason to make the bounding rect 100px bigger on all the sides is to make
    // middle button drag scrolling easier (you can put the image where you want without
    // running out of scene
    auto boundingRect = imagesetEntry->boundingRect();
    boundingRect.adjust(-100, -100, 100, 100);
    scene()->setSceneRect(boundingRect);
}

// Centre position is the position of the centre of the newly created image,
// the newly created image will then 'encapsulate' the centrepoint
void ImagesetVisualMode::createImageEntry(QPointF pos)
{
    // Find an unique image name
    QString name = "NewImage";
    int index = 1;

    while (true)
    {
        ImageEntry* imageEntry = imagesetEntry->getImageEntry(name);
        if (!imageEntry) break;
        name = QString("NewImage_%i").arg(index++);
    }

    QPointF imgPos = pos - QPointF(newImageHalfSize, newImageHalfSize);
    QSizeF imgSize(2.0 * newImageHalfSize, 2.0 * newImageHalfSize);
    QPoint imgOffset(0, 0);
    _editor.getUndoStack()->push(new ImagesetCreateCommand(*this, name, imgPos, imgSize, imgOffset));
}

bool ImagesetVisualMode::moveImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF delta)
{
    if (imageEntries.empty() || delta.manhattanLength() <= 0.0) return false;

    std::vector<ImagesetMoveCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetMoveCommand::Record rec;
        rec.name = imageEntry->name();
        rec.oldPos = imageEntry->pos();
        rec.newPos = imageEntry->pos() + delta;
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetMoveCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::resizeImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF topLeftDelta, QPointF bottomRightDelta)
{
    if (imageEntries.empty() || (topLeftDelta.manhattanLength() <= 0.0 && bottomRightDelta.manhattanLength() <= 0.0)) return false;

    std::vector<ImagesetGeometryChangeCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetGeometryChangeCommand::Record rec;
        rec.name = imageEntry->name();
        rec.oldPos = imageEntry->pos();
        rec.newPos = imageEntry->pos() - topLeftDelta;
        rec.oldRect = imageEntry->rect();
        rec.newRect = imageEntry->rect();
        rec.newRect.setBottomRight(rec.newRect.bottomRight() - topLeftDelta + bottomRightDelta);
        if (rec.newRect.width() < 1.0) rec.newRect.setWidth(1.0);
        if (rec.newRect.height() < 1.0) rec.newRect.setHeight(1.0);
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetGeometryChangeCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::deleteImageEntries(const std::vector<ImageEntry*>& imageEntries)
{
    if (imageEntries.empty()) return false;

    std::vector<ImagesetDeleteCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetDeleteCommand::Record rec;
        rec.name = imageEntry->name();
        rec.pos = imageEntry->pos();
        rec.size = imageEntry->rect().size();
        rec.offset = QPoint(imageEntry->offsetX(), imageEntry->offsetY());
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetDeleteCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::deleteSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    std::vector<ImageEntry*> imageEntries;
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (entry) imageEntries.push_back(entry);
    }

    return deleteImageEntries(imageEntries);
}

bool ImagesetVisualMode::duplicateImageEntries(const std::vector<ImageEntry*>& imageEntries)
{
    if (imageEntries.empty()) return false;

    std::vector<ImagesetDuplicateCommand::Record> undo;
    for (ImageEntry* imageEntry : imageEntries)
    {
        ImagesetDuplicateCommand::Record rec;
        rec.name = getNewImageName(imageEntry->name());
        rec.pos = imageEntry->pos();
        rec.size = imageEntry->rect().size();
        rec.offset = QPoint(imageEntry->offsetX(), imageEntry->offsetY());
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetDuplicateCommand(*this, std::move(undo)));
    return true;
}

bool ImagesetVisualMode::duplicateSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    std::vector<ImageEntry*> imageEntries;
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (entry) imageEntries.push_back(entry);
    }

    return duplicateImageEntries(imageEntries);
}

bool ImagesetVisualMode::cut()
{
    if (!copy()) return false;
    deleteSelectedImageEntries();
    return true;
}

bool ImagesetVisualMode::copy()
{
    auto selection = scene()->selectedItems();

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (QGraphicsItem* item : selection)
    {
        auto entry = dynamic_cast<ImageEntry*>(item);
        if (!entry) continue;

        stream << entry->name();
        stream << entry->pos();
        stream << entry->rect();
        stream << QPoint(entry->offsetX(), entry->offsetY());
    }

    if (!bytes.size()) return false;

    QMimeData* mimeData = new QMimeData;
    mimeData->setData("application/x-ceed-imageset-image-list", bytes);
    QApplication::clipboard()->setMimeData(mimeData);
    return true;
}

bool ImagesetVisualMode::paste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData->hasFormat("application/x-ceed-imageset-image-list")) return false;

    std::vector<ImagesetPasteCommand::Record> undo;
    std::vector<QString> newNames;

    QByteArray bytes = mimeData->data("application/x-ceed-imageset-image-list");
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        QString name;
        stream >> name;

        ImagesetPasteCommand::Record rec;
        rec.name = getNewImageName(name);

        newNames.push_back(rec.name);

        stream >> rec.pos;
        stream >> rec.size;
        stream >> rec.offset;
        undo.push_back(std::move(rec));
    }

    _editor.getUndoStack()->push(new ImagesetPasteCommand(*this, std::move(undo)));

    // Select just the pasted image definitions for convenience
    scene()->clearSelection();
    for (const auto& name : newNames)
        imagesetEntry->getImageEntry(name)->setSelected(true);

    return true;
}

// Returns an image name that is not used in this imageset
QString ImagesetVisualMode::getNewImageName(const QString& desiredName, QString copyPrefix, QString copySuffix)
{
    // Try the desired name exactly
    if (!imagesetEntry->getImageEntry(desiredName)) return desiredName;

    // Try with prefix and suffix
    QString name = copyPrefix + desiredName + copySuffix;
    if (!imagesetEntry->getImageEntry(name)) return name;

    // We're forced to append a counter, start with number 2 (_copy2, copy3, etc.)
    int counter = 2;
    while (true)
    {
        QString indexedName = name + QString::number(counter);
        if (!imagesetEntry->getImageEntry(indexedName)) return indexedName;
        ++counter;
    }
}

bool ImagesetVisualMode::cycleOverlappingImages()
{
    auto selection = scene()->selectedItems();
    if (selection.size() != 1) return false;

    auto rect = selection[0]->boundingRect();
    rect.translate(selection[0]->pos());

    auto overlappingItems = scene()->items(rect);

    // First we stack everything before our current selection
    ImageEntry* successor = nullptr;
    for (QGraphicsItem* item : overlappingItems)
    {
        if (item == selection[0] || item->parentItem() != selection[0]->parentItem())
            continue;

        if (!successor) successor = dynamic_cast<ImageEntry*>(item);
    }

    if (successor)
    {
        for (QGraphicsItem* item : overlappingItems)
        {
            if (item == successor || item->parentItem() != successor->parentItem())
                continue;

            successor->stackBefore(item);
        }

        // We deselect current
        selection[0]->setSelected(false);
        auto selectedImage = dynamic_cast<ImageEntry*>(selection[0]);
        if (selectedImage) selectedImage->hoverLeaveEvent(nullptr);

        // And select what was at the bottom (thus getting this to the top)
        successor->setSelected(true);
        successor->hoverEnterEvent(nullptr);
    }

    return true;
}

void ImagesetVisualMode::slot_selectionChanged()
{
    // If dockWidget is changing the selection, back off
    if (dockWidget->isSelectionUnderway()) return;

    auto selectedItems = scene()->selectedItems();
    if (selectedItems.size() == 1)
    {
        ImageEntry* entry = dynamic_cast<ImageEntry*>(selectedItems[0]);
        dockWidget->scrollToEntry(entry);
    }
}

void ImagesetVisualMode::slot_toggleEditOffsets(bool enabled)
{
    scene()->clearSelection();
    if (imagesetEntry) imagesetEntry->setShowOffsets(enabled);
}

void ImagesetVisualMode::slot_customContextMenu(QPoint point)
{
    contextMenu->exec(mapToGlobal(point));
}

void ImagesetVisualMode::showEvent(QShowEvent* event)
{
    dockWidget->setEnabled(true);
    toolBar->setEnabled(true);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(true);

    setActionsEnabled(true);

    // Call this every time the visual editing is shown to sync all entries up
    slot_toggleEditOffsets(editOffsetsAction->isChecked());

    ResizableGraphicsView::showEvent(event);
}

void ImagesetVisualMode::hideEvent(QHideEvent* event)
{
    setActionsEnabled(false);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(false);

    dockWidget->setEnabled(false);
    toolBar->setEnabled(false);

    ResizableGraphicsView::hideEvent(event);
}

void ImagesetVisualMode::mouseMoveEvent(QMouseEvent* event)
{
    lastCursorPosition = mapToScene(event->pos());
    ResizableGraphicsView::mouseMoveEvent(event);
}

void ImagesetVisualMode::mousePressEvent(QMouseEvent* event)
{
    ResizableGraphicsView::mousePressEvent(event);

    if (event->buttons() & Qt::LeftButton)
    {
        for (QGraphicsItem* selectedItem : scene()->selectedItems())
        {
            auto entry = dynamic_cast<ImageEntry*>(selectedItem);
            if (entry)
            {
                entry->onPotentialMove(true);
                continue;
            }

            auto offset = dynamic_cast<ImageOffsetMark*>(selectedItem);
            if (offset)
            {
                offset->onPotentialMove(true);
                continue;
            }
        }
    }
}

// When mouse is released, we have to check what items were moved and resized.
// AFAIK Qt doesn't give us any move finished notification so I do this manually
void ImagesetVisualMode::mouseReleaseEvent(QMouseEvent* event)
{
    ResizableGraphicsView::mouseReleaseEvent(event);

    std::vector<ImageEntry*> imageEntries;
    std::vector<ImageOffsetMark*> imageOffsets;
    for (QGraphicsItem* selectedItem : scene()->selectedItems())
    {
        auto entry = dynamic_cast<ImageEntry*>(selectedItem);
        if (entry)
        {
            imageEntries.push_back(entry);
            continue;
        }

        auto offset = dynamic_cast<ImageOffsetMark*>(selectedItem);
        if (offset)
        {
            imageOffsets.push_back(offset);
            continue;
        }

        // We have to process parents of resizing handles instead of the handles themselves
        auto handle = dynamic_cast<ResizingHandle*>(selectedItem);
        if (handle)
        {
            imageEntries.push_back(static_cast<ImageEntry*>(handle->parentItem()));
            continue;
        }
    }

    // NOTE: It should never happen that more than one of these sets is populated
    //       User moves images XOR moves offsets XOR resizes images
    //       I don't do elif for robustness though, who knows what can happen ;-)
    std::vector<ImagesetGeometryChangeCommand::Record> resize;
    std::vector<ImagesetMoveCommand::Record> move;
    std::vector<ImagesetOffsetMoveCommand::Record> offsetMove;

    for (ImageEntry* imageEntry : imageEntries)
    {
        auto oldPos = imageEntry->getOldPos();
        if (oldPos.x() > -9999.0) // FIXME: hack for 'not set' position
        {
            // Show the label again if mouse is over because moving finished
            if (imageEntry->isHovered()) imageEntry->showLabel(true);

            // Only include that if the position really changed
            if (oldPos != imageEntry->pos())
            {
                ImagesetMoveCommand::Record rec;
                rec.name = imageEntry->name();
                rec.oldPos = oldPos;
                rec.newPos = imageEntry->pos();
                move.push_back(std::move(rec));
            }
        }

        if (imageEntry->isResized())
        {
            // Only include that if the position or rect really changed
            if (imageEntry->getResizeOldPos() != imageEntry->pos() || imageEntry->getResizeOldRect() != imageEntry->rect())
            {
                ImagesetGeometryChangeCommand::Record rec;
                rec.name = imageEntry->name();
                rec.oldPos = imageEntry->getResizeOldPos();
                rec.newPos = imageEntry->pos();
                rec.oldRect = imageEntry->getResizeOldRect();
                rec.newRect = imageEntry->rect();
                resize.push_back(std::move(rec));
            }
        }

        imageEntry->onPotentialMove(false);
    }

    for (ImageOffsetMark* imageOffset : imageOffsets)
    {
        auto oldPos = imageOffset->getOldPos();
        if (oldPos.x() > -9999.0) // FIXME: hack for 'not set' position
        {
            // Only include that if the position really changed
            if (oldPos != imageOffset->pos())
            {
                ImagesetOffsetMoveCommand::Record rec;
                rec.name = static_cast<ImageEntry*>(imageOffset->parentItem())->name();
                rec.oldPos = oldPos;
                rec.newPos = imageOffset->pos();
                offsetMove.push_back(std::move(rec));
            }
        }

        imageOffset->onPotentialMove(false);
    }

    if (!move.empty())
        _editor.getUndoStack()->push(new ImagesetMoveCommand(*this, std::move(move)));

    if (!resize.empty())
        _editor.getUndoStack()->push(new ImagesetGeometryChangeCommand(*this, std::move(resize)));

    if (!offsetMove.empty())
        _editor.getUndoStack()->push(new ImagesetOffsetMoveCommand(*this, std::move(offsetMove)));
}

// TODO: offset keyboard handling
void ImagesetVisualMode::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    switch (event->key())
    {
        case Qt::Key_Q:
        {
            handled = cycleOverlappingImages();
            break;
        }
        case Qt::Key_Delete:
        {
            handled = deleteSelectedImageEntries();
            break;
        }
        case Qt::Key_A:
        case Qt::Key_D:
        case Qt::Key_W:
        case Qt::Key_S:
        {
            std::vector<ImageEntry*> selection;
            for (QGraphicsItem* item : scene()->selectedItems())
            {
                if (std::find(selection.begin(), selection.end(), item) != selection.end()) continue;

                ImageEntry* entry = dynamic_cast<ImageEntry*>(item);
                if (entry)
                {
                    selection.push_back(entry);
                    continue;
                }

                ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
                if (handle && std::find(selection.begin(), selection.end(), handle->parentItem()) == selection.end())
                {
                    selection.push_back(static_cast<ImageEntry*>(handle->parentItem()));
                    continue;
                }
            }

            if (!selection.empty())
            {
                QPointF delta(0.0, 0.0);
                switch (event->key())
                {
                    case Qt::Key_A: delta.setX(-1.0); break;
                    case Qt::Key_D: delta.setX(1.0); break;
                    case Qt::Key_W: delta.setY(-1.0); break;
                    case Qt::Key_S: delta.setY(1.0); break;
                }

                if (event->modifiers() & Qt::ControlModifier) delta *= 10.0;

                if (event->modifiers() & Qt::ShiftModifier)
                    handled = resizeImageEntries(selection, QPointF(0.0, 0.0), delta);
                else
                    handled = moveImageEntries(selection, delta);
            }

            break;
        }
    }

    if (handled)
        event->accept();
    else
        ResizableGraphicsView::keyReleaseEvent(event);
}
