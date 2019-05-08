#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include <CEGUI/System.h>
#include <CEGUI/GUIContext.h>
#include <qdatetime.h>

static inline bool compareFloat(float a, float b) { return std::fabs(a - b) < 0.0001f; }

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

CEGUIGraphicsScene::CEGUIGraphicsScene(float width, float height)
    : timeOfLastRender(QDateTime::currentMSecsSinceEpoch())
{
    validateResolution(width, height);
    contextWidth = width;
    contextHeight = height;

    const qreal qWidth = static_cast<qreal>(width);
    const qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    auto renderer = static_cast<CEGUI::OpenGLRenderer*>(CEGUI::System::getSingleton().getRenderer());
    auto viewportRect = CEGUI::Rectf(0.f, 0.f, contextWidth, contextHeight);
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, viewportRect);
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

    if (compareFloat(contextWidth, width) && compareFloat(contextHeight, height)) return;

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
