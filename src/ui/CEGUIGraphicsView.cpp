#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "qopenglwidget.h"
#include "qtimer.h"

//!!!subclass of resizable.GraphicsView, cegui.GLContextProvider!
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

/*
    def makeGLContextCurrent(self):
        self.viewport().makeCurrent()

    def mouseMoveEvent(self, event):
        handled = False

        if self.injectInput:
            point = self.mapToScene(QtCore.QPoint(event.x(), event.y()))
            handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMousePosition(point.x(), point.y())

        if not handled:
            super(GraphicsView, self).mouseMoveEvent(event)

    def translateQtMouseButton(self, button):
        ret = None

        if button == QtCore.Qt.LeftButton:
            ret = PyCEGUI.MouseButton.LeftButton
        if button == QtCore.Qt.RightButton:
            ret = PyCEGUI.MouseButton.RightButton

        return ret

    def mousePressEvent(self, event):
        # FIXME: Somehow, if you drag on the Live preview in layout editing on Linux,
        #        it drag moves the whole window

        handled = False

        if self.injectInput:
            button = self.translateQtMouseButton(event.button())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMouseButtonDown(button)

        if not handled:
            super(GraphicsView, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        handled = False

        if self.injectInput:
            button = self.translateQtMouseButton(event.button())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectMouseButtonUp(button)

        if not handled:
            super(GraphicsView, self).mouseReleaseEvent(event)

    def translateQtKeyboardButton(self, button):
        # Shame this isn't standardised :-/ Was a pain to write down

        if button == QtCore.Qt.Key_Escape:
            return PyCEGUI.Key.Escape
        elif button == QtCore.Qt.Key_Tab:
            return PyCEGUI.Key.Tab
        # missing Backtab
        elif button == QtCore.Qt.Key_Backspace:
            return PyCEGUI.Key.Backspace
        elif button in [QtCore.Qt.Key_Return, QtCore.Qt.Key_Enter]:
            return PyCEGUI.Key.Return
        elif button == QtCore.Qt.Key_Insert:
            return PyCEGUI.Key.Insert
        elif button == QtCore.Qt.Key_Delete:
            return PyCEGUI.Key.Delete
        elif button == QtCore.Qt.Key_Pause:
            return PyCEGUI.Key.Pause
        # missing Print
        elif button == QtCore.Qt.Key_SysReq:
            return PyCEGUI.Key.SysRq
        elif button == QtCore.Qt.Key_Home:
            return PyCEGUI.Key.Home
        elif button == QtCore.Qt.Key_End:
            return PyCEGUI.Key.End
        elif button == QtCore.Qt.Key_Left:
            return PyCEGUI.Key.ArrowLeft
        elif button == QtCore.Qt.Key_Up:
            return PyCEGUI.Key.ArrowUp
        elif button == QtCore.Qt.Key_Right:
            return PyCEGUI.Key.ArrowRight
        elif button == QtCore.Qt.Key_Down:
            return PyCEGUI.Key.ArrowDown
        elif button == QtCore.Qt.Key_PageUp:
            return PyCEGUI.Key.PageUp
        elif button == QtCore.Qt.Key_PageDown:
            return PyCEGUI.Key.PageDown
        elif button == QtCore.Qt.Key_Shift:
            return PyCEGUI.Key.LeftShift
        elif button == QtCore.Qt.Key_Control:
            return PyCEGUI.Key.LeftControl
        elif button == QtCore.Qt.Key_Meta:
            return PyCEGUI.Key.LeftWindows
        elif button == QtCore.Qt.Key_Alt:
            return PyCEGUI.Key.LeftAlt
        # missing AltGr
        # missing CapsLock
        # missing NumLock
        # missing ScrollLock
        elif button == QtCore.Qt.Key_F1:
            return PyCEGUI.Key.F1
        elif button == QtCore.Qt.Key_F2:
            return PyCEGUI.Key.F2
        elif button == QtCore.Qt.Key_F3:
            return PyCEGUI.Key.F3
        elif button == QtCore.Qt.Key_F4:
            return PyCEGUI.Key.F4
        elif button == QtCore.Qt.Key_F5:
            return PyCEGUI.Key.F5
        elif button == QtCore.Qt.Key_F6:
            return PyCEGUI.Key.F6
        elif button == QtCore.Qt.Key_F7:
            return PyCEGUI.Key.F7
        elif button == QtCore.Qt.Key_F8:
            return PyCEGUI.Key.F8
        elif button == QtCore.Qt.Key_F9:
            return PyCEGUI.Key.F9
        elif button == QtCore.Qt.Key_F10:
            return PyCEGUI.Key.F10
        elif button == QtCore.Qt.Key_F11:
            return PyCEGUI.Key.F11
        elif button == QtCore.Qt.Key_F12:
            return PyCEGUI.Key.F12
        elif button == QtCore.Qt.Key_F13:
            return PyCEGUI.Key.F13
        elif button == QtCore.Qt.Key_F14:
            return PyCEGUI.Key.F14
        elif button == QtCore.Qt.Key_F15:
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
        elif button == QtCore.Qt.Key_Space:
            return PyCEGUI.Key.Space
        # missing Exclam
        # Qt::Key_QuoteDbl    0x22
        # Qt::Key_NumberSign    0x23
        # Qt::Key_Dollar    0x24
        # Qt::Key_Percent    0x25
        # Qt::Key_Ampersand    0x26
        elif button == QtCore.Qt.Key_Apostrophe:
            return PyCEGUI.Key.Apostrophe
        # Qt::Key_ParenLeft    0x28
        # Qt::Key_ParenRight    0x29
        # Qt::Key_Asterisk    0x2a
        # Qt::Key_Plus    0x2b
        elif button == QtCore.Qt.Key_Comma:
            return PyCEGUI.Key.Comma
        elif button == QtCore.Qt.Key_Minus:
            return PyCEGUI.Key.Minus
        elif button == QtCore.Qt.Key_Period:
            return PyCEGUI.Key.Period
        elif button == QtCore.Qt.Key_Slash:
            return PyCEGUI.Key.Slash
        elif button == QtCore.Qt.Key_0:
            return PyCEGUI.Key.Zero
        elif button == QtCore.Qt.Key_1:
            return PyCEGUI.Key.One
        elif button == QtCore.Qt.Key_2:
            return PyCEGUI.Key.Two
        elif button == QtCore.Qt.Key_3:
            return PyCEGUI.Key.Three
        elif button == QtCore.Qt.Key_4:
            return PyCEGUI.Key.Four
        elif button == QtCore.Qt.Key_5:
            return PyCEGUI.Key.Five
        elif button == QtCore.Qt.Key_6:
            return PyCEGUI.Key.Six
        elif button == QtCore.Qt.Key_7:
            return PyCEGUI.Key.Seven
        elif button == QtCore.Qt.Key_8:
            return PyCEGUI.Key.Eight
        elif button == QtCore.Qt.Key_9:
            return PyCEGUI.Key.Nine
        elif button == QtCore.Qt.Key_Colon:
            return PyCEGUI.Key.Colon
        elif button == QtCore.Qt.Key_Semicolon:
            return PyCEGUI.Key.Semicolon
        # missing Key_Less
        elif button == QtCore.Qt.Key_Equal:
            return PyCEGUI.Key.Equals
        # missing Key_Greater
        # missing Key_Question
        elif button == QtCore.Qt.Key_At:
            return PyCEGUI.Key.At
        elif button == QtCore.Qt.Key_A:
            return PyCEGUI.Key.A
        elif button == QtCore.Qt.Key_B:
            return PyCEGUI.Key.B
        elif button == QtCore.Qt.Key_C:
            return PyCEGUI.Key.C
        elif button == QtCore.Qt.Key_D:
            return PyCEGUI.Key.D
        elif button == QtCore.Qt.Key_E:
            return PyCEGUI.Key.E
        elif button == QtCore.Qt.Key_F:
            return PyCEGUI.Key.F
        elif button == QtCore.Qt.Key_G:
            return PyCEGUI.Key.G
        elif button == QtCore.Qt.Key_H:
            return PyCEGUI.Key.H
        elif button == QtCore.Qt.Key_I:
            return PyCEGUI.Key.I
        elif button == QtCore.Qt.Key_J:
            return PyCEGUI.Key.J
        elif button == QtCore.Qt.Key_K:
            return PyCEGUI.Key.K
        elif button == QtCore.Qt.Key_L:
            return PyCEGUI.Key.L
        elif button == QtCore.Qt.Key_M:
            return PyCEGUI.Key.M
        elif button == QtCore.Qt.Key_N:
            return PyCEGUI.Key.N
        elif button == QtCore.Qt.Key_O:
            return PyCEGUI.Key.O
        elif button == QtCore.Qt.Key_P:
            return PyCEGUI.Key.P
        elif button == QtCore.Qt.Key_Q:
            return PyCEGUI.Key.Q
        elif button == QtCore.Qt.Key_R:
            return PyCEGUI.Key.R
        elif button == QtCore.Qt.Key_S:
            return PyCEGUI.Key.S
        elif button == QtCore.Qt.Key_T:
            return PyCEGUI.Key.T
        elif button == QtCore.Qt.Key_U:
            return PyCEGUI.Key.U
        elif button == QtCore.Qt.Key_V:
            return PyCEGUI.Key.V
        elif button == QtCore.Qt.Key_W:
            return PyCEGUI.Key.W
        elif button == QtCore.Qt.Key_X:
            return PyCEGUI.Key.X
        elif button == QtCore.Qt.Key_Y:
            return PyCEGUI.Key.Y
        elif button == QtCore.Qt.Key_Z:
            return PyCEGUI.Key.Z

        # The rest are weird keys I refuse to type here

    def keyPressEvent(self, event):
        handled = False

        if self.injectInput:
            button = self.translateQtKeyboardButton(event.key())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectKeyDown(button)

            char = event.text()
            if len(char) > 0:
                handled = handled or PyCEGUI.System.getSingleton().getDefaultGUIContext().injectChar(ord(char[0]))

        if not handled:
            super(GraphicsView, self).keyPressEvent(event)

    def keyReleaseEvent(self, event):
        handled = False

        if self.injectInput:
            button = self.translateQtKeyboardButton(event.key())

            if button is not None:
                handled = PyCEGUI.System.getSingleton().getDefaultGUIContext().injectKeyUp(button)

        if not handled:
            super(GraphicsView, self).keyPressEvent(event)
*/
