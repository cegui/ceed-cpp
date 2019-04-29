#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
#include "src/util/Utils.h"
#include "src/Application.h"
#include <CEGUI/System.h>
#include <CEGUI/WindowManager.h>
#include "qopenglwidget.h"
#include "qpaintengine.h"
#include "qopenglframebufferobject.h"
#include "qopengltextureblitter.h"
#include "qopenglcontext.h"
#include "qopenglfunctions.h"
#include "qtimer.h"
#include "qevent.h"

CEGUIGraphicsView::CEGUIGraphicsView(QWidget *parent) :
    ResizableGraphicsView(parent)
{
    QSurfaceFormat format;
    format.setSamples(0);

    auto vp = new QOpenGLWidget();
    vp->setFormat(format);
    setViewport(vp);
    setViewportUpdateMode(FullViewportUpdate);

    setOptimizationFlags(DontClipPainter | DontAdjustForAntialiasing);

    // Prepare to receive input
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const auto checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    const auto checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    const auto checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    const auto checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();

    checkerboardBrush = Utils::getCheckerboardBrush(checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

    blitter = new QOpenGLTextureBlitter();
}

CEGUIGraphicsView::~CEGUIGraphicsView()
{
    auto vp = static_cast<QOpenGLWidget*>(viewport());
    vp->makeCurrent();
    delete blitter;
    vp->doneCurrent();
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

// We override this and draw CEGUI instead of the whole background.
// This method uses a FBO to implement zooming, scrolling around, etc...
// FBOs are therefore required by CEED and it won't run without a GPU that supports them.
void CEGUIGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    auto painterType = painter->paintEngine()->type();
    if (painterType != QPaintEngine::OpenGL && painterType != QPaintEngine::OpenGL2)
    {
        qWarning("CEGUIGraphicsView::drawBackground() > needs a QOpenGLWidget viewport on the graphics view");
        return;
    }

    auto ceguiScene = static_cast<CEGUIGraphicsScene*>(scene());
    if (!ceguiScene) return;

    const int contextWidth = static_cast<int>(ceguiScene->getContextWidth());
    const int contextHeight = static_cast<int>(ceguiScene->getContextHeight());
    QRect viewportRect(0, 0, contextWidth, contextHeight);
    painter->setPen(Qt::transparent);
    painter->setBrush(checkerboardBrush);
    painter->drawRect(viewportRect);

    painter->beginNativePainting();

    // We have to render to FBO and then scale/translate that since CEGUI doesn't allow
    // scaling the whole rendering root directly

    if (!fbo || fbo->size().width() != contextWidth || fbo->size().height() != contextHeight)
    {
        if (fbo) delete fbo;
        fbo = new QOpenGLFramebufferObject(viewportRect.size());
    }

    fbo->bind();

    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glClearColor(0.f, 0.f, 0.f, 0.f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto renderer = CEGUI::System::getSingleton().getRenderer();
    renderer->beginRendering();
    ceguiScene->drawCEGUIContext();
    renderer->endRendering();

    fbo->release();

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!blitter->isCreated()) blitter->create();

    blitter->bind();
    const QRect fboRect(QPoint(0, 0), fbo->size());
    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(fboRect, rect.toRect());
    blitter->blit(fbo->texture(), target, QOpenGLTextureBlitter::OriginBottomLeft);
    blitter->release();

    painter->endNativePainting();

    CEGUI::WindowManager::getSingleton().cleanDeadPool();

    if (continuousRendering)
    {
        if (continuousRenderingTargetFPS <= 0)
        {
            updateSelfAndScene();
        }
        else
        {
            const float lastDelta = static_cast<float>(ceguiScene->getLastDelta());
            const float frameTime = 1.0f / static_cast<float>(continuousRenderingTargetFPS);

            if (frameTime > lastDelta)
            {
                // * 1000 because QTimer thinks in milliseconds
                QTimer::singleShot(static_cast<int>((frameTime - lastDelta) * 1000.f), this, SLOT(updateSelfAndScene()));
            }
            else
            {
                updateSelfAndScene();
            }
        }
    }
}

void CEGUIGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    bool handled = false;

    if (_injectInput)
    {
        QPointF point = mapToScene(event->pos());
/*
            handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMousePosition(point.x(), point.y())
*/
    }

    if (!handled) ResizableGraphicsView::mouseMoveEvent(event);
}

// FIXME: Somehow, if you drag on the Live preview in layout editing on Linux, it drag moves the whole window
void CEGUIGraphicsView::mousePressEvent(QMouseEvent* event)
{
    bool handled = false;

    if (_injectInput)
    {
/*
            button = self.translateQtMouseButton(event.button())
            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMouseButtonDown(button)
*/
    }

    if (!handled) ResizableGraphicsView::mousePressEvent(event);
}

void CEGUIGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    bool handled = false;

    if (_injectInput)
    {
/*
            button = self.translateQtMouseButton(event.button())
            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMouseButtonUp(button)
*/
    }

    if (!handled) ResizableGraphicsView::mouseReleaseEvent(event);
}

void CEGUIGraphicsView::keyPressEvent(QKeyEvent* event)
{
    bool handled = false;

    if (_injectInput)
    {
/*
            button = self.translateQtKeyboardButton(event.key())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectKeyDown(button)

            char = event.text()
            if len(char) > 0:
                handled = handled or PyCEGUI.System.getSingleton().getDefaultGUIContext().injectChar(ord(char[0]))
*/
    }

    if (!handled) ResizableGraphicsView::keyPressEvent(event);
}

void CEGUIGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    if (_injectInput)
    {
/*
            button = self.translateQtKeyboardButton(event.key())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectKeyUp(button)
*/
    }

    if (!handled) ResizableGraphicsView::keyReleaseEvent(event);
}

/*
    def translateQtMouseButton(self, button):
        ret = None

        if button == Qt::LeftButton:
            ret = PyCEGUI.MouseButton.LeftButton
        if button == Qt::RightButton:
            ret = PyCEGUI.MouseButton.RightButton

        return ret

    def translateQtKeyboardButton(self, button):
        // Shame this isn't standardised :-/ Was a pain to write down

        if button == Qt::Key_Escape:
            return PyCEGUI.Key.Escape
        elif button == Qt::Key_Tab:
            return PyCEGUI.Key.Tab
        # missing Backtab
        elif button == Qt::Key_Backspace:
            return PyCEGUI.Key.Backspace
        elif button in [Qt::Key_Return, Qt::Key_Enter]:
            return PyCEGUI.Key.Return
        elif button == Qt::Key_Insert:
            return PyCEGUI.Key.Insert
        elif button == Qt::Key_Delete:
            return PyCEGUI.Key.Delete
        elif button == Qt::Key_Pause:
            return PyCEGUI.Key.Pause
        # missing Print
        elif button == Qt::Key_SysReq:
            return PyCEGUI.Key.SysRq
        elif button == Qt::Key_Home:
            return PyCEGUI.Key.Home
        elif button == Qt::Key_End:
            return PyCEGUI.Key.End
        elif button == Qt::Key_Left:
            return PyCEGUI.Key.ArrowLeft
        elif button == Qt::Key_Up:
            return PyCEGUI.Key.ArrowUp
        elif button == Qt::Key_Right:
            return PyCEGUI.Key.ArrowRight
        elif button == Qt::Key_Down:
            return PyCEGUI.Key.ArrowDown
        elif button == Qt::Key_PageUp:
            return PyCEGUI.Key.PageUp
        elif button == Qt::Key_PageDown:
            return PyCEGUI.Key.PageDown
        elif button == Qt::Key_Shift:
            return PyCEGUI.Key.LeftShift
        elif button == Qt::Key_Control:
            return PyCEGUI.Key.LeftControl
        elif button == Qt::Key_Meta:
            return PyCEGUI.Key.LeftWindows
        elif button == Qt::Key_Alt:
            return PyCEGUI.Key.LeftAlt
        # missing AltGr
        # missing CapsLock
        # missing NumLock
        # missing ScrollLock
        elif button == Qt::Key_F1:
            return PyCEGUI.Key.F1
        elif button == Qt::Key_F2:
            return PyCEGUI.Key.F2
        elif button == Qt::Key_F3:
            return PyCEGUI.Key.F3
        elif button == Qt::Key_F4:
            return PyCEGUI.Key.F4
        elif button == Qt::Key_F5:
            return PyCEGUI.Key.F5
        elif button == Qt::Key_F6:
            return PyCEGUI.Key.F6
        elif button == Qt::Key_F7:
            return PyCEGUI.Key.F7
        elif button == Qt::Key_F8:
            return PyCEGUI.Key.F8
        elif button == Qt::Key_F9:
            return PyCEGUI.Key.F9
        elif button == Qt::Key_F10:
            return PyCEGUI.Key.F10
        elif button == Qt::Key_F11:
            return PyCEGUI.Key.F11
        elif button == Qt::Key_F12:
            return PyCEGUI.Key.F12
        elif button == Qt::Key_F13:
            return PyCEGUI.Key.F13
        elif button == Qt::Key_F14:
            return PyCEGUI.Key.F14
        elif button == Qt::Key_F15:
            return PyCEGUI.Key.F15
        # missing F16 - F35
        # Qt::Key_Super_L    0x01000053
        # Qt::Key_Super_R    0x01000054
        # Qt::Key_Menu    0x01000055
        # Qt::Key_Hyper_L    0x01000056
        # Qt::Key_Hyper_R    0x01000057
        # Qt::Key_Help    0x01000058
        # Qt::Key_Direction_L    0x01000059
        # Qt::Key_Direction_R    0x01000060
        elif button == Qt::Key_Space:
            return PyCEGUI.Key.Space
        # missing Exclam
        # Qt::Key_QuoteDbl    0x22
        # Qt::Key_NumberSign    0x23
        # Qt::Key_Dollar    0x24
        # Qt::Key_Percent    0x25
        # Qt::Key_Ampersand    0x26
        elif button == Qt::Key_Apostrophe:
            return PyCEGUI.Key.Apostrophe
        # Qt::Key_ParenLeft    0x28
        # Qt::Key_ParenRight    0x29
        # Qt::Key_Asterisk    0x2a
        # Qt::Key_Plus    0x2b
        elif button == Qt::Key_Comma:
            return PyCEGUI.Key.Comma
        elif button == Qt::Key_Minus:
            return PyCEGUI.Key.Minus
        elif button == Qt::Key_Period:
            return PyCEGUI.Key.Period
        elif button == Qt::Key_Slash:
            return PyCEGUI.Key.Slash
        elif button == Qt::Key_0:
            return PyCEGUI.Key.Zero
        elif button == Qt::Key_1:
            return PyCEGUI.Key.One
        elif button == Qt::Key_2:
            return PyCEGUI.Key.Two
        elif button == Qt::Key_3:
            return PyCEGUI.Key.Three
        elif button == Qt::Key_4:
            return PyCEGUI.Key.Four
        elif button == Qt::Key_5:
            return PyCEGUI.Key.Five
        elif button == Qt::Key_6:
            return PyCEGUI.Key.Six
        elif button == Qt::Key_7:
            return PyCEGUI.Key.Seven
        elif button == Qt::Key_8:
            return PyCEGUI.Key.Eight
        elif button == Qt::Key_9:
            return PyCEGUI.Key.Nine
        elif button == Qt::Key_Colon:
            return PyCEGUI.Key.Colon
        elif button == Qt::Key_Semicolon:
            return PyCEGUI.Key.Semicolon
        # missing Key_Less
        elif button == Qt::Key_Equal:
            return PyCEGUI.Key.Equals
        # missing Key_Greater
        # missing Key_Question
        elif button == Qt::Key_At:
            return PyCEGUI.Key.At
        elif button == Qt::Key_A:
            return PyCEGUI.Key.A
        elif button == Qt::Key_B:
            return PyCEGUI.Key.B
        elif button == Qt::Key_C:
            return PyCEGUI.Key.C
        elif button == Qt::Key_D:
            return PyCEGUI.Key.D
        elif button == Qt::Key_E:
            return PyCEGUI.Key.E
        elif button == Qt::Key_F:
            return PyCEGUI.Key.F
        elif button == Qt::Key_G:
            return PyCEGUI.Key.G
        elif button == Qt::Key_H:
            return PyCEGUI.Key.H
        elif button == Qt::Key_I:
            return PyCEGUI.Key.I
        elif button == Qt::Key_J:
            return PyCEGUI.Key.J
        elif button == Qt::Key_K:
            return PyCEGUI.Key.K
        elif button == Qt::Key_L:
            return PyCEGUI.Key.L
        elif button == Qt::Key_M:
            return PyCEGUI.Key.M
        elif button == Qt::Key_N:
            return PyCEGUI.Key.N
        elif button == Qt::Key_O:
            return PyCEGUI.Key.O
        elif button == Qt::Key_P:
            return PyCEGUI.Key.P
        elif button == Qt::Key_Q:
            return PyCEGUI.Key.Q
        elif button == Qt::Key_R:
            return PyCEGUI.Key.R
        elif button == Qt::Key_S:
            return PyCEGUI.Key.S
        elif button == Qt::Key_T:
            return PyCEGUI.Key.T
        elif button == Qt::Key_U:
            return PyCEGUI.Key.U
        elif button == Qt::Key_V:
            return PyCEGUI.Key.V
        elif button == Qt::Key_W:
            return PyCEGUI.Key.W
        elif button == Qt::Key_X:
            return PyCEGUI.Key.X
        elif button == Qt::Key_Y:
            return PyCEGUI.Key.Y
        elif button == Qt::Key_Z:
            return PyCEGUI.Key.Z

        // The rest are weird keys I refuse to type here
*/
