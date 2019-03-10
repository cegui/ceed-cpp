#ifndef CEGUIGRAPHICSSCENE_H
#define CEGUIGRAPHICSSCENE_H

#include "qgraphicsscene.h"

// A scene that draws CEGUI as it's background.
// Subclass this to be able to show Qt graphics items and widgets
// on top of the embedded CEGUI widget!
// Interaction is also supported.

class CEGUIGraphicsScene : public QGraphicsScene
{
public:

    CEGUIGraphicsScene();

    time_t getLastDelta() const { return lastDelta; }
    void setCEGUIDisplaySize(float width, float height, bool lazyUpdate = true);

    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

protected:

    time_t lastDelta = 0;
    time_t timeOfLastRender;

    float padding = 100.f;
    float contextWidth = 0.f;
    float contextHeight = 0.f;
};

#endif // CEGUIGRAPHICSSCENE_H
