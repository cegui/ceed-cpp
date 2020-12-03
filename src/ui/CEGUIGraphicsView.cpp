#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
#include "src/util/SettingsEntry.h"
#include "src/util/Utils.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/Application.h"
#include <CEGUI/System.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/InputAggregator.h>
#include <CEGUI/GUIContext.h>
#include "qopenglwidget.h"
#include "qpaintengine.h"
#include "qopenglframebufferobject.h"
#include "qopengltextureblitter.h"
#include "qopenglcontext.h"
#include "qopenglfunctions.h"
#include "qtimer.h"
#include "qevent.h"

CEGUIGraphicsView::CEGUIGraphicsView(QWidget *parent) :
    ResizableGraphicsView(parent)
{
    // FIXME QTBUG: Qt 5.13.0 text rendering in OpenGL breaks on QOpenGLWidget delete
    setViewport(qobject_cast<Application*>(qApp)->getMainWindow()->allocateOpenGLWidget());

    //setViewport(new QOpenGLWidget());
    setViewportUpdateMode(FullViewportUpdate);

    setOptimizationFlags(DontClipPainter | DontAdjustForAntialiasing);

    // Prepare to receive input
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    blitter = new QOpenGLTextureBlitter();

    updateCheckerboardBrush();

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    connect(settings->getEntry("cegui/background/checker_width"), &SettingsEntry::valueChanged, this, &CEGUIGraphicsView::updateCheckerboardBrush);
    connect(settings->getEntry("cegui/background/checker_height"), &SettingsEntry::valueChanged, this, &CEGUIGraphicsView::updateCheckerboardBrush);
    connect(settings->getEntry("cegui/background/first_colour"), &SettingsEntry::valueChanged, this, &CEGUIGraphicsView::updateCheckerboardBrush);
    connect(settings->getEntry("cegui/background/second_colour"), &SettingsEntry::valueChanged, this, &CEGUIGraphicsView::updateCheckerboardBrush);
}

CEGUIGraphicsView::~CEGUIGraphicsView()
{
    if (ceguiInput) delete ceguiInput;

    auto vp = static_cast<QOpenGLWidget*>(viewport());
    vp->makeCurrent();
    delete blitter;
    vp->doneCurrent();

    // FIXME QTBUG: Qt 5.13.0 text rendering in OpenGL breaks on QOpenGLWidget delete
    qobject_cast<Application*>(qApp)->getMainWindow()->freeOpenGLWidget(viewport());
}

// FIXME: works only for the current scene because we can't catch scene change event
// (Or maybe we can but I have no internet right now to check it)
void CEGUIGraphicsView::injectInput(bool inject)
{
    assert(scene());

    _injectInput = inject;

    if (_injectInput && scene())
    {
        if (ceguiInput) delete ceguiInput;
        auto ctx = static_cast<CEGUIGraphicsScene*>(scene())->getCEGUIContext();
        ceguiInput = new CEGUI::InputAggregator(ctx);
        ceguiInput->initialise();
        ceguiInput->setMouseClickEventGenerationEnabled(true);
    }
}

// We override this and draw CEGUI instead of the whole background.
// This method uses a FBO to implement zooming, scrolling around, etc...
// FBOs are therefore required by CEED and it won't run without a GPU that supports them.
void CEGUIGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    auto painterType = painter->paintEngine()->type();
    if (painterType != QPaintEngine::OpenGL && painterType != QPaintEngine::OpenGL2)
    {
        qWarning("CEGUIGraphicsView::drawBackground() > needs a QOpenGLWidget viewport on the graphics view");
        return;
    }

    auto ceguiScene = static_cast<CEGUIGraphicsScene*>(scene());
    if (!ceguiScene) return;

    const int contextWidth = static_cast<int>(ceguiScene->getContextWidth());
    const int contextHeight = static_cast<int>(ceguiScene->getContextHeight());
    QRect viewportRect(0, 0, contextWidth, contextHeight);
    painter->setPen(Qt::transparent);
    painter->setBrush(checkerboardBrush);
    painter->drawRect(viewportRect);

    // Store view's OpenGL context
    QOpenGLContext* currContext = QOpenGLContext::currentContext();
    QSurface* currSurface = currContext->surface();
    currContext->doneCurrent();

    // The main part of CEGUI rendering. Note that it is rendered in an OpenGL context
    // where its resources are created, since some of them aren't shareable (like VAOs).
    // Resulting texture is shared instead and is blitted to screen in view's context.
    ceguiScene->drawCEGUIContextOffscreen();

    // Restore view's OpenGL context
    currContext->makeCurrent(currSurface);

    painter->beginNativePainting();

    // Restore parameters possibly affected by CEGUI renderer and prepare to blitting
    auto gl = currContext->functions();
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glViewport(0, 0, viewport()->width(), viewport()->height());

    if (!blitter->isCreated()) blitter->create();

    QOpenGLFramebufferObject* fbo = ceguiScene->getOffscreenBuffer();
    if (fbo)
    {
        blitter->bind();
        const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(viewportRect, rect.toRect());
        blitter->blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
        blitter->release();
    }

    painter->endNativePainting();

    CEGUI::WindowManager::getSingleton().cleanDeadPool();

    if (continuousRendering)
    {
        constexpr float desiredFPS = 60.f;
        constexpr float frameTimeMsec = (desiredFPS <= 0.f) ? 0.f : (1000.0f / desiredFPS);

        const float lastDeltaMSec = static_cast<float>(ceguiScene->getLastDeltaMSec()); //???per-view?
        const qint64 msecLeft = std::max(static_cast<qint64>(0), static_cast<qint64>(frameTimeMsec - lastDeltaMSec));

        // Direct call to update() from inside here doesn't seem to work, so we schedule a
        // timer signal even if the time of the next frame has come.
        QTimer::singleShot(msecLeft, [this]()
        {
            update();
            if (scene())
                scene()->update();
        });
    }
}

void CEGUIGraphicsView::updateCheckerboardBrush()
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const auto checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    const auto checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    const auto checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    const auto checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();

    checkerboardBrush = Utils::getCheckerboardBrush(checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

    if (scene()) scene()->update();
}

void CEGUIGraphicsView::wheelEvent(QWheelEvent* event)
{
    bool handled = false;

    if (_injectInput && ceguiInput)
    {
        handled = ceguiInput->injectMouseWheelChange(static_cast<float>(event->delta()));
    }

    if (!handled) ResizableGraphicsView::wheelEvent(event);
}

void CEGUIGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    bool handled = false;

    QPointF point = mapToScene(event->pos());
    emit cursorPositionChanged(static_cast<int>(point.x()), static_cast<int>(point.y()));

    if (_injectInput && ceguiInput)
    {
        handled = ceguiInput->injectMousePosition(static_cast<float>(point.x()), static_cast<float>(point.y()));
    }

    if (!handled) ResizableGraphicsView::mouseMoveEvent(event);
}

// FIXME: Somehow, if you drag on the Live preview in layout editing on Linux, it drag moves the whole window
void CEGUIGraphicsView::mousePressEvent(QMouseEvent* event)
{
    bool handled = false;

    if (_injectInput && ceguiInput)
    {
        auto button = CEGUIUtils::qtMouseButtonToMouseButton(event->button());
        handled = ceguiInput->injectMouseButtonDown(button);
    }

    if (!handled) ResizableGraphicsView::mousePressEvent(event);
}

void CEGUIGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    bool handled = false;

    if (_injectInput && ceguiInput)
    {
        auto button = CEGUIUtils::qtMouseButtonToMouseButton(event->button());
        handled = ceguiInput->injectMouseButtonUp(button);
    }

    if (!handled) ResizableGraphicsView::mouseReleaseEvent(event);
}

void CEGUIGraphicsView::keyPressEvent(QKeyEvent* event)
{
    bool handled = false;

    if (_injectInput && ceguiInput)
    {
        auto key = CEGUIUtils::qtKeyToKey(event->key());
        if (key != CEGUI::Key::Scan::Unknown)
            handled = ceguiInput->injectKeyDown(key);

        auto text = event->text();
        if (!text.isEmpty())
            handled |= ceguiInput->injectChar(text[0].unicode());
    }

    if (!handled) ResizableGraphicsView::keyPressEvent(event);
}

void CEGUIGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    if (_injectInput && ceguiInput)
    {
        auto key = CEGUIUtils::qtKeyToKey(event->key());
        if (key != CEGUI::Key::Scan::Unknown)
            handled = ceguiInput->injectKeyUp(key);
    }

    if (!handled) ResizableGraphicsView::keyReleaseEvent(event);
}
