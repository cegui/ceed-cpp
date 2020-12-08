#ifndef CEGUIGRAPHICSSCENE_H
#define CEGUIGRAPHICSSCENE_H

#include "qgraphicsscene.h"

// A scene that draws CEGUI as it's background. Subclass this to be able to show Qt graphic
// items and widgets on top of the embedded CEGUI widget! Interaction is also supported.

namespace CEGUI
{
    class GUIContext;
}

class QOpenGLFramebufferObject;

class CEGUIGraphicsScene : public QGraphicsScene
{
public:

    CEGUIGraphicsScene(QObject* parent = nullptr, float width = 0.f, float height = 0.f);
    virtual ~CEGUIGraphicsScene() override;

    virtual void setCEGUIDisplaySize(float width, float height);
    void drawCEGUIContextOffscreen();
    QImage getCEGUIScreenshot();

    qint64 getLastDeltaMSec() const { return lastDelta; }
    CEGUI::GUIContext* getCEGUIContext() const { return ceguiContext; }
    float getContextWidth() const { return contextWidth; }
    float getContextHeight() const { return contextHeight; }
    QList<QGraphicsItem*> topLevelItems() const;

    QOpenGLFramebufferObject* getOffscreenBuffer() const { return _fbo; }

    bool ensureDefaultFontExists();

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    void drawCEGUIContextInternal();

    CEGUI::GUIContext* ceguiContext = nullptr;
    QOpenGLFramebufferObject* _fbo = nullptr;

    qint64 lastDelta = 0;
    qint64 timeOfLastRender;

    qreal padding = 30.0;
    float contextWidth = 0.f;
    float contextHeight = 0.f;
};

#endif // CEGUIGRAPHICSSCENE_H
