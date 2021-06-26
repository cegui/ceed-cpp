#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/editors/imageset/ImagesetUndoCommands.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/imageset/ImageEntry.h"
#include "src/ui/imageset/ImageOffsetMark.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/ui/ResizingHandle.h"
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include "qclipboard.h"
#include "qmimedata.h"
#include "qtoolbar.h"
#include "qevent.h"
#include "qmenu.h"
#include "qdom.h"
#include <qrubberband.h>

constexpr qreal newImageHalfSize = 25.0;

ImagesetVisualMode::ImagesetVisualMode(MultiModeEditor& editor)
    : IEditMode(editor)
    , _lastCursorScenePos(newImageHalfSize, newImageHalfSize)
{
    wheelZoomEnabled = true;
    middleButtonDragScrollEnabled = true;

    setScene(new QGraphicsScene());

    setFocusPolicy(Qt::ClickFocus);
    setFrameStyle(QFrame::NoFrame);

    setRenderHint(QPainter::TextAntialiasing, true);

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

        // FIXME QTBUG: Qt 5.13.0 text rendering in OpenGL breaks on QOpenGLWidget delete
        setViewport(qobject_cast<Application*>(qApp)->getMainWindow()->allocateOpenGLWidget());

        //setViewport(new QOpenGLWidget());
        setViewportUpdateMode(FullViewportUpdate);
    }

    setDragMode(RubberBandDrag);
    setBackgroundBrush(QBrush(Qt::lightGray));

    connect(scene(), &QGraphicsScene::selectionChanged, this, &ImagesetVisualMode::slot_selectionChanged);

    dockWidget = new ImagesetEditorDockWidget(*this);

    Application* app = qobject_cast<Application*>(qApp);

    editOffsetsAction = app->getAction("imageset/edit_offsets");
    cycleOverlappingAction = app->getAction("imageset/cycle_overlapping");
    createImageAction = app->getAction("imageset/create_image");
    duplicateSelectedImagesAction = app->getAction("imageset/duplicate_image");
    focusImageListFilterBoxAction = app->getAction("imageset/focus_image_list_filter_box");
    //app->setActionsEnabled("imageset", false);

    auto mainWindow = app->getMainWindow();

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

ImagesetVisualMode::~ImagesetVisualMode()
{
    // FIXME QTBUG: Qt 5.13.0 text rendering in OpenGL breaks on QOpenGLWidget delete
    qobject_cast<Application*>(qApp)->getMainWindow()->freeOpenGLWidget(viewport());

    // Order matters!
    delete imagesetEntry;
    delete dockWidget;
}

void ImagesetVisualMode::activate(MainWindow& mainWindow, bool editorActivated)
{
    IEditMode::activate(mainWindow, editorActivated);

    dockWidget->setEnabled(true);
    mainWindow.getToolbar("Imageset")->setEnabled(true);

    mainWindow.setEditorMenuEnabled(true);

    createActiveStateConnections();

    // Call this every time the visual editing is shown to sync all entries up
    slot_toggleEditOffsets(editOffsetsAction->isChecked());

    setFocus();
}

bool ImagesetVisualMode::deactivate(MainWindow& mainWindow, bool editorDeactivated)
{
    disconnectActiveStateConnections();

    mainWindow.setEditorMenuEnabled(false);

    dockWidget->setEnabled(false);
    mainWindow.getToolbar("Imageset")->setEnabled(false);

    return IEditMode::deactivate(mainWindow, editorDeactivated);
}

void ImagesetVisualMode::createActiveStateConnections()
{
    _activeStateConnections.push_back(connect(editOffsetsAction, &QAction::toggled, this, &ImagesetVisualMode::slot_toggleEditOffsets));
    _activeStateConnections.push_back(connect(cycleOverlappingAction, &QAction::triggered, this, &ImagesetVisualMode::cycleOverlappingImages));
    _activeStateConnections.push_back(connect(createImageAction, &QAction::triggered, this, &ImagesetVisualMode::createImageEntryAtCursor));
    _activeStateConnections.push_back(connect(duplicateSelectedImagesAction, &QAction::triggered, this, &ImagesetVisualMode::duplicateSelectedImageEntries));
    _activeStateConnections.push_back(connect(focusImageListFilterBoxAction, &QAction::triggered, dockWidget, &ImagesetEditorDockWidget::focusImageListFilterBox));
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

void ImagesetVisualMode::createImageEntry(const QRectF& rect)
{
    // Find an unique image name
    QString name = "NewImage";
    int index = 1;

    while (true)
    {
        ImageEntry* imageEntry = imagesetEntry->getImageEntry(name);
        if (!imageEntry) break;
        name = QString("NewImage_%1").arg(index++);
    }

    QPoint imgOffset(0, 0);
    _editor.getUndoStack()->push(new ImagesetCreateCommand(*this, name, rect.topLeft(), rect.size(), imgOffset));

    // Select just created entry for convenience
    if (auto createdEntry = imagesetEntry->getImageEntry(name))
    {
        scene()->clearSelection();
        createdEntry->setSelected(true);
    }
}

// Centre position is the position of the centre of the newly created image,
// the newly created image will then 'encapsulate' the centrepoint
void ImagesetVisualMode::createImageEntry(QPointF pos)
{
    const QPointF imgPos = pos - QPointF(newImageHalfSize, newImageHalfSize);
    const QSizeF imgSize(2.0 * newImageHalfSize, 2.0 * newImageHalfSize);
    createImageEntry(QRectF(imgPos, imgSize));
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

    QMimeData* mimeData = new QMimeData();
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

    // Select just pasted image definitions for convenience
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

QPoint ImagesetVisualMode::roundToImagePixels(QPoint src) const
{
    return mapFromScene(mapToScene(src).toPoint());
}

void ImagesetVisualMode::mouseMoveEvent(QMouseEvent* event)
{
    _lastCursorScenePos = mapToScene(event->pos());

    if (_rubberBand && _rubberBand->isVisible())
    {
        _rubberBand->setGeometry(QRect(_mouseDownPos, roundToImagePixels(event->pos())).normalized());
        event->accept();
        return;
    }
//    rect = QRectF(QPointF(round(rect.topLeft().x()), round(rect.topLeft().y())),
//                  QPointF(round(rect.bottomRight().x()), round(rect.bottomRight().y())));

    ResizableGraphicsView::mouseMoveEvent(event);
}

void ImagesetVisualMode::mousePressEvent(QMouseEvent* event)
{
    // Ctrl+Drag to create new image rect with a rubber band
    if ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier))
    {
        _mouseDownPos = roundToImagePixels(event->pos());
        if (!_rubberBand) _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        _rubberBand->setGeometry(QRect(_mouseDownPos, QSize()));
        _rubberBand->show();
        event->accept();
        return;
    }

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
    if (_rubberBand && _rubberBand->isVisible())
    {
        _rubberBand->hide();
        if (_rubberBand->geometry().isValid())
            createImageEntry(mapToScene(_rubberBand->geometry()).boundingRect().toRect());
        event->accept();
        return;
    }

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
            if (imageEntry->getResizeStartPos() != imageEntry->pos() || imageEntry->getResizeStartRect() != imageEntry->rect())
            {
                ImagesetGeometryChangeCommand::Record rec;
                rec.name = imageEntry->name();
                rec.oldPos = imageEntry->getResizeStartPos();
                rec.newPos = imageEntry->pos();
                rec.oldRect = imageEntry->getResizeStartRect();
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
    if (event->matches(QKeySequence::SelectAll))
    {
        scene()->clearSelection();
        for (auto item : items())
            if (dynamic_cast<ImageEntry*>(item))
                item->setSelected(true);
        handled = true;
    }
    else switch (event->key())
    {
        case Qt::Key_Control:
        {
            if (_rubberBand && _rubberBand->isVisible())
            {
                _rubberBand->hide();
                handled = true;
            }
            break;
        }
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
        case Qt::Key_Escape:
        {
            if (!scene()->selectedItems().isEmpty())
            {
                scene()->clearSelection();
                handled = true;
            }
            break;
        }
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
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
                    case Qt::Key_Left: delta.setX(-1.0); break;
                    case Qt::Key_Right: delta.setX(1.0); break;
                    case Qt::Key_Up: delta.setY(-1.0); break;
                    case Qt::Key_Down: delta.setY(1.0); break;
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
