#ifndef CEGUIGRAPHICSSCENE_H
#define CEGUIGRAPHICSSCENE_H

#include "qgraphicsscene.h"

// A scene that draws CEGUI as it's background.
// Subclass this to be able to show Qt graphics items and widgets
// on top of the embedded CEGUI widget!
// Interaction is also supported.

namespace CEGUI
{
    class GUIContext;
}

class QOpenGLFramebufferObject;

class CEGUIGraphicsScene : public QGraphicsScene
{
public:

    CEGUIGraphicsScene();
    virtual ~CEGUIGraphicsScene() override;

    time_t getLastDelta() const { return lastDelta; }
    CEGUI::GUIContext* getCEGUIContext() const { return ceguiContext; }
    virtual void setCEGUIDisplaySize(float width, float height, bool lazyUpdate = true);

    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

protected:

    QOpenGLFramebufferObject* fbo = nullptr;

    CEGUI::GUIContext* ceguiContext = nullptr;

    time_t lastDelta = 0;
    time_t timeOfLastRender;

    qreal padding = 30.0;
    float contextWidth = 0.f;
    float contextHeight = 0.f;

    QBrush checkerboardBrush;
    QColor checkerFirstColour = Qt::darkGray;
    QColor checkerSecondColour = Qt::gray;
    int checkerWidth = 5;
    int checkerHeight = 5;
};

#endif // CEGUIGRAPHICSSCENE_H
