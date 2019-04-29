#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
#include "src/util/Utils.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/Application.h"
#include <CEGUI/System.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include "qpainter.h"
#include "qpaintengine.h"
#include "qopenglframebufferobject.h"
#include "qopengltextureblitter.h"
#include "qopenglcontext.h"
#include "qopenglfunctions.h"

CEGUIGraphicsScene::CEGUIGraphicsScene()
    : timeOfLastRender(time(nullptr))
    , contextWidth(800.f)
    , contextHeight(600.f)
{
    auto renderer = static_cast<CEGUI::OpenGLRenderer*>(CEGUI::System::getSingleton().getRenderer());
    auto viewportRect = CEGUI::Rectf(0.f, 0.f, contextWidth, contextHeight);
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, viewportRect);
    ceguiContext = &CEGUI::System::getSingleton().createGUIContext(*renderTarget);

    setSceneRect(QRectF(-padding, -padding, static_cast<qreal>(contextWidth) + 2.0 * padding, static_cast<qreal>(contextHeight) + 2.0 * padding));

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();

    checkerboardBrush = Utils::getCheckerboardBrush(checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

    blitter = new QOpenGLTextureBlitter();
}

CEGUIGraphicsScene::~CEGUIGraphicsScene()
{
    //!!!FIXME: call destroy() manually when creation context is current!
    //!!!must be in a view, as context belongs to view! many views require many blitters!
    delete blitter;

    if (ceguiContext)
    {
        auto renderTarget = &ceguiContext->getRenderTarget();
        CEGUI::System::getSingleton().destroyGUIContext(*ceguiContext);
        delete renderTarget;
    }
}

static inline bool compareFloat(float a, float b) { return std::fabs(a - b) < 0.0001f; }

void CEGUIGraphicsScene::setCEGUIDisplaySize(float width, float height)
{
    if (compareFloat(contextWidth, width) && compareFloat(contextHeight, height)) return;

    contextWidth = width;
    contextHeight = height;

    qreal qWidth = static_cast<qreal>(width);
    qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    CEGUI::Sizef newSize(contextWidth, contextHeight);
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(newSize);

    auto& renderTarget = ceguiContext->getRenderTarget();
    auto area = renderTarget.getArea();
    area.setSize(newSize);
    renderTarget.setArea(area);

    // This makes sure the FBO is the correct size
    if (fbo)
    {
        delete fbo;
        fbo = nullptr;
    }
}

// We override this and draw CEGUI instead of the whole background.
// This method uses a FBO to implement zooming, scrolling around, etc...
// FBOs are therefore required by CEED and it won't run without a GPU that supports them.
void CEGUIGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    // Be robust, this is usually caused by recursive repainting
    if (!painter->paintEngine())
    {
        // TODO: this assert is to check is this the case in Qt 5 still.
        //       Remove it (or the entire check) after testing.
        assert(false);
        return;
    }

    auto painterType = painter->paintEngine()->type();
    if (painterType != QPaintEngine::OpenGL && painterType != QPaintEngine::OpenGL2)
    {
        qWarning("CEGUIGraphicsScene::drawBackground() > needs a QOpenGLWidget viewport on the graphics view");
        return;
    }

    auto currTime = time(nullptr);
    lastDelta = currTime - timeOfLastRender;
    timeOfLastRender = currTime;

    // Inject the time passed since the last render all at once
    CEGUI::System::getSingleton().injectTimePulse(lastDelta);
    ceguiContext->injectTimePulse(lastDelta);

    QRect viewport(0, 0, static_cast<int>(contextWidth), static_cast<int>(contextHeight));
    painter->setPen(Qt::transparent);
    painter->setBrush(checkerboardBrush);
    painter->drawRect(viewport);

    painter->beginNativePainting();

    // We have to render to FBO and then scale/translate that since CEGUI doesn't allow
    // scaling the whole rendering root directly

    if (!fbo) fbo = new QOpenGLFramebufferObject(viewport.size());

    fbo->bind();

    auto glContext = QOpenGLContext::currentContext();

    auto gl = glContext->functions();
    gl->glClearColor(0.f, 0.f, 0.f, 0.f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto renderer = CEGUI::System::getSingleton().getRenderer();
    renderer->beginRendering();
    ceguiContext->draw();
    renderer->endRendering();

    fbo->release();

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!blitter->isCreated()) blitter->create();

    blitter->bind();
    const QRect fboRect(QPoint(0.f, 0.f), fbo->size());
    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(fboRect, rect.toRect());
    blitter->blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
    blitter->release();

    painter->endNativePainting();

    CEGUI::WindowManager::getSingleton().cleanDeadPool();
}
