#include "src/ui/CEGUIGraphicsScene.h"
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include <CEGUI/System.h>
#include <CEGUI/GUIContext.h>

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

    update();
}

void CEGUIGraphicsScene::drawCEGUIContext()
{
    if (!ceguiContext) return;

    auto currTime = time(nullptr);
    lastDelta = currTime - timeOfLastRender;
    timeOfLastRender = currTime;

    // Inject the time passed since the last render all at once
    CEGUI::System::getSingleton().injectTimePulse(lastDelta);
    ceguiContext->injectTimePulse(lastDelta);

    ceguiContext->draw();
}
