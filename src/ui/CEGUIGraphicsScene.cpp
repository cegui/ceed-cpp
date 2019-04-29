#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
#include "src/util/Utils.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/Application.h"
#include "qpainter.h"
#include "qpaintengine.h"
#include "qopenglframebufferobject.h"
#include "qopenglcontext.h"
#include "qopenglfunctions.h"
#include "qopenglfunctions_2_0.h"
#include <ctime>

CEGUIGraphicsScene::CEGUIGraphicsScene()
    : timeOfLastRender(time(nullptr))
{
    setCEGUIDisplaySize(800, 600, true);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();

    checkerboardBrush = Utils::getCheckerboardBrush(checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);
}

void CEGUIGraphicsScene::setCEGUIDisplaySize(float width, float height, bool lazyUpdate)
{
    contextWidth = width;
    contextHeight = height;

    qreal qWidth = static_cast<qreal>(width);
    qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    if (!lazyUpdate)
        ; /* PyCEGUI.System.getSingleton().notifyDisplaySizeChanged(self.ceguiDisplaySize);
          */

    // FIXME: only if size changed!
    if (fbo)
    {
        delete fbo;
        fbo = nullptr;
    }
}

// We override this and draw CEGUI instead of the whole background.
// This method uses a FBO to implement zooming, scrolling around, etc...
// FBOs are therefore required by CEED and it won't run without a GPU that supports them.
void CEGUIGraphicsScene::drawBackground(QPainter* painter, const QRectF&)
{
    //???!!!use const QRectF& rect arg?!

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
        qWarning("cegui.GraphicsScene: drawBackground needs a "
                 "QOpenGLWidget to be set as a viewport on the graphics view");
        return;
    }

    auto currTime = time(nullptr);
    lastDelta = currTime - timeOfLastRender;
    timeOfLastRender = currTime;

    // Inject the time passed since the last render all at once
/*
    system = PyCEGUI.System.getSingleton()
    self.ceguiInstance.lastRenderTimeDelta = self.lastDelta
    system.injectTimePulse(self.lastDelta)
    system.getDefaultGUIContext().injectTimePulse(self.lastDelta)
*/

    painter->setPen(Qt::transparent);
    painter->setBrush(checkerboardBrush);
    painter->drawRect(0, 0, static_cast<int>(contextWidth), static_cast<int>(contextHeight));

    painter->beginNativePainting();

    CEGUIProjectManager::Instance().ensureCEGUIInitialized();
/*
    if self.ceguiDisplaySize != PyCEGUI.System.getSingleton().getRenderer().getDisplaySize():
        # FIXME: Change when multi root is in CEGUI core
        PyCEGUI.System.getSingleton().notifyDisplaySizeChanged(self.ceguiDisplaySize)

    # markAsDirty is called on the default GUI context to work around potential issues with dangling
    # references in the rendering code for some versions of CEGUI.
    system.getDefaultGUIContext().markAsDirty()
*/
    // We have to render to FBO and then scale/translate that since CEGUI doesn't allow
    // scaling the whole rendering root directly

    // This makes sure the FBO is the correct size
    if (!fbo)
    {
        QSize desiredSize(static_cast<int>(std::ceil(contextWidth)), static_cast<int>(std::ceil(contextHeight)));
        fbo = new QOpenGLFramebufferObject(desiredSize);
    }

    fbo->bind();

    auto glContext = QOpenGLContext::currentContext();

    //???use default subset? need to use VBO, no fixed function glBegin / glEnd!
    //auto gl = glContext->functions();
    auto* gl = glContext->versionFunctions<QOpenGLFunctions_2_0>();
    if (!gl)
    {
        qWarning("cegui.GraphicsScene: required OpenGL 2.0 not supported");
        return;
    }
    gl->glClearColor(0.f, 0.f, 0.f, 0.f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
    system.renderAllGUIContexts() //!!!render only the current context if possible (should be)!
*/

    fbo->release();

    // The stretch and translation should be done automatically by QPainter at this point so just this code will do
    gl->glActiveTexture(GL_TEXTURE0);

    gl->glEnable(GL_TEXTURE_2D);
    gl->glBindTexture(GL_TEXTURE_2D, fbo->texture());

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: I was told that this is the slowest method to draw with OpenGL,
    //       with which I certainly agree.
    //
    //       No profiling has been done at all and I don't suspect this to be
    //       a painful performance problem.
    //
    //       Still, changing this to a less pathetic rendering method would be great.

    gl->glBegin(GL_TRIANGLES);

    // Top left
    gl->glTexCoord2f(0.f, 1.f);
    gl->glVertex3f(0.f, 0.f, 0.f);

    // Top right
    gl->glTexCoord2f(1.f, 1.f);
    gl->glVertex3f(fbo->size().width(), 0.f, 0.f);

    // Bottom right
    gl->glTexCoord2f(1.f, 0.f);
    gl->glVertex3f(fbo->size().width(), fbo->size().height(), 0.f);

    // Bottom right
    gl->glTexCoord2f(1, 0.f);
    gl->glVertex3f(fbo->size().width(), fbo->size().height(), 0.f);

    // Bottom left
    gl->glTexCoord2f(0.f, 0.f);
    gl->glVertex3f(0.f, fbo->size().height(), 0.f);

    // Top left
    gl->glTexCoord2f(0.f, 1);
    gl->glVertex3f(0.f, 0.f, 0.f);

    /*
    system.getDefaultGUIContext().markAsDirty()
    */

    gl->glEnd();

    painter->endNativePainting();
}
