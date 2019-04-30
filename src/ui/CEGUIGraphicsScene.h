#ifndef CEGUIGRAPHICSSCENE_H
#define CEGUIGRAPHICSSCENE_H

#include "qgraphicsscene.h"

// A scene that draws CEGUI as it's background. Subclass this to be able to show Qt graphic
// items and widgets on top of the embedded CEGUI widget! Interaction is also supported.

namespace CEGUI
{
    class GUIContext;
}

class CEGUIGraphicsScene : public QGraphicsScene
{
public:

    CEGUIGraphicsScene(float width = 0.f, float height = 0.f);
    virtual ~CEGUIGraphicsScene() override;

    virtual void setCEGUIDisplaySize(float width, float height);
    void drawCEGUIContext();

    time_t getLastDelta() const { return lastDelta; }
    CEGUI::GUIContext* getCEGUIContext() const { return ceguiContext; }
    float getContextWidth() const { return contextWidth; }
    float getContextHeight() const { return contextHeight; }

protected:

    CEGUI::GUIContext* ceguiContext = nullptr;

    time_t lastDelta = 0;
    time_t timeOfLastRender;

    qreal padding = 30.0;
    float contextWidth = 0.f;
    float contextHeight = 0.f;
};

#endif // CEGUIGRAPHICSSCENE_H
