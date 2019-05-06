#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
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
    QSurfaceFormat format;
    format.setSamples(0);

    auto vp = new QOpenGLWidget();
    vp->setFormat(format);
    setViewport(vp);
    setViewportUpdateMode(FullViewportUpdate);

    setOptimizationFlags(DontClipPainter | DontAdjustForAntialiasing);

    // Prepare to receive input
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const auto checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    const auto checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    const auto checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    const auto checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();

    checkerboardBrush = Utils::getCheckerboardBrush(checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

    blitter = new QOpenGLTextureBlitter();
}

CEGUIGraphicsView::~CEGUIGraphicsView()
{
    if (ceguiInput) delete ceguiInput;

    auto vp = static_cast<QOpenGLWidget*>(viewport());
    vp->makeCurrent();
    delete blitter;
    vp->doneCurrent();
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

// FIXME: now in Qt5 this doesn't do what it is intended to do!
void CEGUIGraphicsView::causeFullRedraw()
{
    update();
    if (scene())
        scene()->update();
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

    painter->beginNativePainting();

    // We have to render to FBO and then scale/translate that since CEGUI doesn't allow
    // scaling the whole rendering root directly

    if (!fbo || fbo->size().width() != contextWidth || fbo->size().height() != contextHeight)
    {
        if (fbo) delete fbo;
        fbo = new QOpenGLFramebufferObject(viewportRect.size());
    }

    fbo->bind();

    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glClearColor(0.f, 0.f, 0.f, 0.f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto renderer = CEGUI::System::getSingleton().getRenderer();
    renderer->beginRendering();
    ceguiScene->drawCEGUIContext();
    renderer->endRendering();

    fbo->release();

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!blitter->isCreated()) blitter->create();

    blitter->bind();
    const QRect fboRect(QPoint(0, 0), fbo->size());
    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(fboRect, rect.toRect());
    blitter->blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
    blitter->release();

    painter->endNativePainting();

    CEGUI::WindowManager::getSingleton().cleanDeadPool();

    if (continuousRendering)
    {
        if (continuousRenderingTargetFPS <= 0)
        {
            causeFullRedraw();
        }
        else
        {
            const float lastDelta = static_cast<float>(ceguiScene->getLastDelta());
            const float frameTime = 1.0f / static_cast<float>(continuousRenderingTargetFPS);

            if (frameTime > lastDelta)
            {
                // * 1000 because QTimer thinks in milliseconds
                QTimer::singleShot(static_cast<int>((frameTime - lastDelta) * 1000.f), this, &CEGUIGraphicsView::causeFullRedraw);
            }
            else
            {
                causeFullRedraw();
            }
        }
    }
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
