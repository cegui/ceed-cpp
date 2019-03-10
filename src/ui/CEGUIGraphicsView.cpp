#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "qopenglwidget.h"
#include "qtimer.h"

// TODO: look at resizable.GraphicsView for scrolling & scaling
CEGUIGraphicsView::CEGUIGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    auto vp = new QOpenGLWidget();

    // Mainly to tone down potential antialiasing
    // TODO: look if glEnable(GL_MULTISAMPLE) call is required
    QSurfaceFormat format;
    format.setSamples(2);
    vp->setFormat(format);

    setViewport(vp);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontAdjustForAntialiasing);

    // Prepare to receive input
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
}

CEGUIGraphicsView::~CEGUIGraphicsView()
{
}

void CEGUIGraphicsView::injectInput(bool inject)
{
    _injectInput = inject;
}

void CEGUIGraphicsView::updateSelfAndScene()
{
    update();
    if (scene())
        scene()->update();
}

void CEGUIGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if (continuousRendering)
    {
        if (continuousRenderingTargetFPS <= 0)
        {
            updateSelfAndScene();
        }
        else
        {
            const CEGUIGraphicsScene* pScene = static_cast<const CEGUIGraphicsScene*>(scene());
            const float lastDelta = pScene ? static_cast<float>(pScene->getLastDelta()) : 0.f;
            const float frameTime = 1.0f / static_cast<float>(continuousRenderingTargetFPS);

            if (frameTime > lastDelta)
            {
                // * 1000 because QTimer thinks in milliseconds
                QTimer::singleShot(static_cast<int>(frameTime - lastDelta) * 1000,
                                         this, SLOT(updateSelfAndScene));
            }
            else
            {
                updateSelfAndScene();
            }
        }
    }
    else
    {
        // We don't mark ourselves as dirty if user didn't request continuous rendering
    }
}
