#include "src/ui/CEGUIWidget.h"
#include "ui_CEGUIWidget.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"

CEGUIWidget::CEGUIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CEGUIWidget)
{
    ui->setupUi(this);

    ui->view->setBackgroundRole(QPalette::Dark);

    /*
        ui->view->containerWidget = this;
        self.debugInfo = DebugInfo(self)
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
    // In Qt4 was:
    //CEGUIGraphicsView* view = findChild<CEGUIGraphicsView*>("view");
    //view->viewport()->makeCurrent();
}

// If you have already activated this container, you can call this to enable CEGUI input propagation
// (The CEGUI instance associated will get mouse and keyboard events if the widget has focus)
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

void CEGUIWidget::on_resolutionBox_editTextChanged(const QString& arg1)
{
    auto text = ui->resolutionBox->currentText();
    if (text == "Project default")
    {
        // Special case
        auto project = CEGUIProjectManager::Instance().getCurrentProject();
        if (!project) return;
        text = project->defaultResolution;
    }

/*
        res = text.split("x", 1)
        if len(res) == 2:
            try:
                # clamp both to 1 - 4096, should suit 99% of all cases
                width = max(1, min(4096, int(res[0])))
                height = max(1, min(4096, int(res[1])))

                makeOpenGLContextCurrent()
                self.view.scene().setCEGUIDisplaySize(width, height, lazyUpdate = False)

            except ValueError:
                # ignore invalid literals
                pass
*/
}

/*

class ViewState(object):
    def __init__(self):
        self.transform = None
        self.horizontalScroll = 0
        self.verticalScroll = 0
//////

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
        currentParentWidget = self.parentWidget()
        if currentParentWidget is not None:
            self.deactivate(currentParentWidget)

        currentParentWidget = parentWidget

        if scene is None:
            scene = qtgraphics.GraphicsScene(self.ceguiInstance)

        currentParentWidget.setUpdatesEnabled(False)
        self.view.setScene(scene)
        # make sure the resolution is set right for the given scene
        self.slot_resolutionBoxChanged(self.resolutionBox.currentText())

        if currentParentWidget.layout():
            currentParentWidget.layout().addWidget(self)
        else:
            self.setParent(currentParentWidget)
        currentParentWidget.setUpdatesEnabled(True)

        # cause full redraw of the default context to ensure that nothing gets stuck
        PyCEGUI.System.getSingleton().getDefaultGUIContext().markAsDirty()

        # and mark the view as dirty to force Qt to redraw it
        self.view.update()

        # finally, set the OpenGL context for CEGUI as current as other code may rely on it
        makeOpenGLContextCurrent()

    def deactivate(self, parentWidget):
        """Deactivates the widget from use in given parentWidget (QWidget derived class)
        see activate

        Note: We strive to be very robust about various differences across platforms (the order in which hide/show events
        are triggered, etc...), so we automatically deactivate if activating with a preexisting parentWidget. That's the
        reason for the parentWidget parameter.
        """

        currentParentWidget = self.parentWidget()
        if currentParentWidget != parentWidget:
            return

        currentParentWidget.setUpdatesEnabled(False)
        # back to the defaults
        self.setViewFeatures()
        self.view.setScene(None)

        if currentParentWidget.layout():
            currentParentWidget.layout().removeWidget(self)
        else:
            self.setParent(None)
        currentParentWidget.setUpdatesEnabled(True)

        currentParentWidget = None

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
*/
