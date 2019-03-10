#include "src/ui/CEGUIGraphicsView.h"
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
    //
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
            // * 1000 because QTimer thinks in milliseconds
            float lastDelta = 0.f; //scene() ? scene()->lastDelta : 0.f;
            QTimer::singleShot(std::max(0.f, ((1.0f / continuousRenderingTargetFPS) - lastDelta) * 1000.f),
                                     this, SLOT(updateSelfAndScene));
        }
    }
    else
    {
        // We don't mark ourselves as dirty if user didn't request continuous rendering
    }
}
