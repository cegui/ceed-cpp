#include "src/ui/CEGUIWidget.h"
#include "ui_CEGUIWidget.h"

CEGUIWidget::CEGUIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CEGUIWidget)
{
    ui->setupUi(this);

    /*
        self.currentParentWidget = None

        self.debugInfo = DebugInfo(self)
        self.view = self.findChild(qtgraphics.GraphicsView, "view")
        self.ceguiInstance.setGLContextProvider(self.view)
        self.view.setBackgroundRole(QtGui.QPalette.Dark)
        self.view.containerWidget = self

        self.resolutionBox = self.findChild(QtGui.QComboBox, "resolutionBox")
        self.resolutionBox.editTextChanged.connect(self.slot_resolutionBoxChanged)

        self.debugInfoButton = self.findChild(QtGui.QPushButton, "debugInfoButton")
        self.debugInfoButton.clicked.connect(self.slot_debugInfoButton)
    */
}

CEGUIWidget::~CEGUIWidget()
{
    delete ui;
}

void CEGUIWidget::makeOpenGLContextCurrent()
{
    // Do need in this form in Qt5?
    assert(false);

    //???accept context as arg?
    //CEGUIGraphicsView* view = findChild<CEGUIGraphicsView*>("view");
    //view->viewport()->makeCurrent();
}

void CEGUIWidget::setInputEnabled(bool enable)
{
    CEGUIGraphicsView* view = findChild<CEGUIGraphicsView*>("view");
    if (view)
        view->injectInput(enable);
}

void CEGUIWidget::on_debugInfoButton_clicked()
{
    //self.debugInfo.show()
}

/*
    def setViewFeatures(self, wheelZoom = False, middleButtonScroll = False, continuousRendering = True):
        """The CEGUI view class has several enable/disable features that are very hard to achieve using
        inheritance/composition so they are kept in the CEGUI view class and its base class.

        This method enables/disables various features, calling it with no parameters switches to default.

        wheelZoom - mouse wheel will zoom in and out
        middleButtonScroll - pressing and dragging with the middle button will cause panning/scrolling
        continuousRendering - CEGUI will render continuously (not just when you tell it to)
        """

        # always zoom to the original 100% when changing view features
        self.view.zoomOriginal()
        self.view.wheelZoomEnabled = wheelZoom

        self.view.middleButtonDragScrollEnabled = middleButtonScroll

        self.view.continuousRendering = continuousRendering

    def activate(self, parentWidget, scene = None):
        """Activates the CEGUI Widget for the given parentWidget (QWidget derived class).
        """

        # sometimes things get called in the opposite order, lets be forgiving and robust!
        if self.currentParentWidget is not None:
            self.deactivate(self.currentParentWidget)

        self.currentParentWidget = parentWidget

        if scene is None:
            scene = qtgraphics.GraphicsScene(self.ceguiInstance)

        self.currentParentWidget.setUpdatesEnabled(False)
        self.view.setScene(scene)
        # make sure the resolution is set right for the given scene
        self.slot_resolutionBoxChanged(self.resolutionBox.currentText())

        if self.currentParentWidget.layout():
            self.currentParentWidget.layout().addWidget(self)
        else:
            self.setParent(self.currentParentWidget)
        self.currentParentWidget.setUpdatesEnabled(True)

        # cause full redraw of the default context to ensure that nothing gets stuck
        PyCEGUI.System.getSingleton().getDefaultGUIContext().markAsDirty()

        # and mark the view as dirty to force Qt to redraw it
        self.view.update()

        # finally, set the OpenGL context for CEGUI as current as other code may rely on it
        self.makeGLContextCurrent()

    def deactivate(self, parentWidget):
        """Deactivates the widget from use in given parentWidget (QWidget derived class)
        see activate

        Note: We strive to be very robust about various differences across platforms (the order in which hide/show events
        are triggered, etc...), so we automatically deactivate if activating with a preexisting parentWidget. That's the
        reason for the parentWidget parameter.
        """

        if self.currentParentWidget != parentWidget:
            return

        self.currentParentWidget.setUpdatesEnabled(False)
        # back to the defaults
        self.setViewFeatures()
        self.view.setScene(None)

        if self.currentParentWidget.layout():
            self.currentParentWidget.layout().removeWidget(self)
        else:
            self.setParent(None)
        self.currentParentWidget.setUpdatesEnabled(True)

        self.currentParentWidget = None

    def updateResolution(self):
        text = self.resolutionBox.currentText()

        if text == "Project default":
            # special case
            project = self.mainWindow.project

            if project is not None:
                text = project.CEGUIDefaultResolution

        res = text.split("x", 1)
        if len(res) == 2:
            try:
                # clamp both to 1 - 4096, should suit 99% of all cases
                width = max(1, min(4096, int(res[0])))
                height = max(1, min(4096, int(res[1])))

                self.ceguiInstance.makeGLContextCurrent()
                self.view.scene().setCEGUIDisplaySize(width, height, lazyUpdate = False)

            except ValueError:
                # ignore invalid literals
                pass

    def setViewState(self, viewState):
        self.view.setTransform(viewState.transform)
        self.view.horizontalScrollBar().setValue(viewState.horizontalScroll)
        self.view.verticalScrollBar().setValue(viewState.verticalScroll)

    def getViewState(self):
        ret = ViewState()
        ret.transform = self.view.transform()
        ret.horizontalScroll = self.view.horizontalScrollBar().value()
        ret.verticalScroll = self.view.verticalScrollBar().value()

        return ret

    def slot_resolutionBoxChanged(self, _):
        self.updateResolution()
*/
