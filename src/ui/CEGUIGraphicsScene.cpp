#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include <CEGUI/RendererModules/OpenGL/RendererBase.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include <CEGUI/System.h>
#include <CEGUI/GUIContext.h>
#include <qdatetime.h>
#include <qgraphicsitem.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qopenglframebufferobject.h>

static void validateResolution(float& width, float& height)
{
    if (width < 1.f || height < 1.f)
    {
        auto project = CEGUIManager::Instance().getCurrentProject();
        if (project)
        {
            width = project->getDefaultResolution().width();
            height = project->getDefaultResolution().height();
        }
        else
        {
            width = 1280.f;
            height = 720.f;
        }
    }
}

CEGUIGraphicsScene::CEGUIGraphicsScene(QObject* parent, float width, float height)
    : QGraphicsScene(parent)
    , timeOfLastRender(QDateTime::currentMSecsSinceEpoch())
{
    validateResolution(width, height);
    contextWidth = width;
    contextHeight = height;

    const qreal qWidth = static_cast<qreal>(width);
    const qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    auto renderer = static_cast<CEGUI::OpenGLRendererBase*>(CEGUI::System::getSingleton().getRenderer());
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, CEGUI::Rectf(0.f, 0.f, contextWidth, contextHeight));
    ceguiContext = &CEGUI::System::getSingleton().createGUIContext(*renderTarget);
}

CEGUIGraphicsScene::~CEGUIGraphicsScene()
{
    if (ceguiContext)
    {
        auto renderTarget = &ceguiContext->getRenderTarget();
        CEGUI::System::getSingleton().destroyGUIContext(*ceguiContext);
        delete renderTarget;
    }
}

void CEGUIGraphicsScene::setCEGUIDisplaySize(float width, float height)
{
    validateResolution(width, height);

    if (qFuzzyCompare(contextWidth, width) && qFuzzyCompare(contextHeight, height)) return;

    contextWidth = width;
    contextHeight = height;

    const qreal qWidth = static_cast<qreal>(width);
    const qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    CEGUI::Sizef newSize(contextWidth, contextHeight);
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(newSize);

    auto& renderTarget = ceguiContext->getRenderTarget();
    auto area = renderTarget.getArea();
    area.setSize(newSize);
    renderTarget.setArea(area);

    update();
}

void CEGUIGraphicsScene::drawCEGUIContext()
{
    if (!ceguiContext) return;

    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
    lastDelta = currTime - timeOfLastRender;
    timeOfLastRender = currTime;

    // Inject the time passed since the last render all at once
    CEGUI::System::getSingleton().injectTimePulse(lastDelta);
    ceguiContext->injectTimePulse(lastDelta);

    ceguiContext->draw();
}

QImage CEGUIGraphicsScene::getCEGUIScreenshot() const
{
    if (!ceguiContext) return QImage();

    CEGUIManager::Instance().ensureCEGUIInitialized();

    CEGUIManager::Instance().makeOpenGLContextCurrent();

    auto fbo = new QOpenGLFramebufferObject(static_cast<int>(contextWidth), static_cast<int>(contextHeight));
    fbo->bind();

    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glClearColor(0.f, 0.f, 0.f, 0.f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto renderer = CEGUI::System::getSingleton().getRenderer();
    renderer->beginRendering();
    ceguiContext->draw();
    renderer->endRendering();

    fbo->release();

    QImage result = fbo->toImage();

    delete fbo;

    CEGUIManager::Instance().doneOpenGLContextCurrent();

    return result;
}

QList<QGraphicsItem*> CEGUIGraphicsScene::topLevelItems() const
{
    QList<QGraphicsItem*> ret;
    for (QGraphicsItem* item : items())
    {
        if (!item->parentItem())
            ret.push_back(item);
    }

    return ret;
}
