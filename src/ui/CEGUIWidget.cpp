#include "src/ui/CEGUIWidget.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "ui_CEGUIWidget.h"
#include "qscrollbar.h"

CEGUIWidget::CEGUIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CEGUIWidget)
{
    ui->setupUi(this);

    ui->view->setBackgroundRole(QPalette::Dark);

    /*
        self.debugInfo = DebugInfo(self)
    */
}

CEGUIWidget::~CEGUIWidget()
{
    delete ui;
}

void CEGUIWidget::setScene(CEGUIGraphicsScene* scene)
{
    ui->view->setScene(scene);

    // Make sure the resolution is set right for the given scene
    on_resolutionBox_editTextChanged(ui->resolutionBox->currentText());

    // And mark the view as dirty to force Qt to redraw it
    ui->view->update();
}

CEGUIGraphicsScene* CEGUIWidget::getScene() const
{
    return static_cast<CEGUIGraphicsScene*>(ui->view->scene());
}

// Activates the CEGUI Widget for the given parentWidget (QWidget derived class)
void CEGUIWidget::activate(QWidget* newParent, CEGUIGraphicsScene* scene)
{
    // Sometimes things get called in the opposite order, lets be forgiving and robust!
    auto currentParentWidget = parentWidget();
    if (currentParentWidget)
        deactivate(currentParentWidget);

    currentParentWidget = newParent;

    if (!scene) scene = new CEGUIGraphicsScene();

    currentParentWidget->setUpdatesEnabled(false);

    ui->view->setScene(scene);

    // Make sure the resolution is set right for the given scene
    on_resolutionBox_editTextChanged(ui->resolutionBox->currentText());

    if (currentParentWidget->layout())
        currentParentWidget->layout()->addWidget(this);
    else
        setParent(currentParentWidget);

    currentParentWidget->setUpdatesEnabled(true);

/*
        // Cause full redraw of the default context to ensure that nothing gets stuck
        CEGUI::System::getSingleton().getDefaultGUIContext().markAsDirty()
*/

    // And mark the view as dirty to force Qt to redraw it
    ui->view->update();

    // Finally, set the OpenGL context for CEGUI as current as other code may rely on it
    //makeOpenGLContextCurrent();
    assert(false);
}

// Deactivates the widget from use in given parentWidget (QWidget derived class), see activate
// Note: We strive to be very robust about various differences across platforms (the order in which hide/show events
// are triggered, etc...), so we automatically deactivate if activating with a preexisting parentWidget. That's the
// reason for the parentWidget parameter.
void CEGUIWidget::deactivate(QWidget* oldParent)
{
    auto currentParentWidget = parentWidget();
    if (currentParentWidget != oldParent) return;

    currentParentWidget->setUpdatesEnabled(false);

    // Back to the defaults
    setViewFeatures();
    ui->view->setScene(nullptr);

    if (currentParentWidget->layout())
        currentParentWidget->layout()->removeWidget(this);
    else
        setParent(nullptr);

    currentParentWidget->setUpdatesEnabled(true);
}

// The CEGUI view class has several enable/disable features that are very hard to achieve using
// inheritance/composition so they are kept in the CEGUI view class and its base class.
// This method enables/disables various features, calling it with no parameters switches to default.
// wheelZoom - mouse wheel will zoom in and out
// middleButtonScroll - pressing and dragging with the middle button will cause panning/scrolling
// continuousRendering - CEGUI will render continuously (not just when you tell it to)
void CEGUIWidget::setViewFeatures(bool wheelZoom, bool middleButtonScroll, bool continuousRendering)
{
    // Always zoom to the original 100% when changing view features
    ui->view->zoomReset();

    /*
    ui->view->wheelZoomEnabled = wheelZoom

    ui->view->middleButtonDragScrollEnabled = middleButtonScroll
    */

    ui->view->setContinuousRendering(continuousRendering);
}

// If you have already activated this container, you can call this to enable CEGUI input propagation
// (The CEGUI instance associated will get mouse and keyboard events if the widget has focus)
void CEGUIWidget::setInputEnabled(bool enable)
{
    if (ui->view)
        ui->view->injectInput(enable);
}

CEGUIWidget::ViewState CEGUIWidget::getViewState() const
{
    CEGUIWidget::ViewState state;
    state.transform = ui->view->transform();
    state.horizontalScroll = ui->view->horizontalScrollBar()->value();
    state.verticalScroll = ui->view->verticalScrollBar()->value();
    return state;
}

void CEGUIWidget::setViewState(const CEGUIWidget::ViewState& state)
{
    ui->view->setTransform(state.transform);
    ui->view->horizontalScrollBar()->setValue(state.horizontalScroll);
    ui->view->verticalScrollBar()->setValue(state.verticalScroll);
}

void CEGUIWidget::on_debugInfoButton_clicked()
{
    //self.debugInfo.show()
}

void CEGUIWidget::on_resolutionBox_editTextChanged(const QString& arg1)
{
    int width = 0;
    int height = 0;

    auto text = ui->resolutionBox->currentText();
    if (text != "Project default") // Special case, leave zeroes for default
    {
        auto sepPos = text.indexOf('x');
        if (sepPos < 0) return;

        // Clamp both to 1 - 4096, should suit 99% of all cases
        bool ok = false;
        width = std::max(1, std::min(4096, text.leftRef(sepPos).toInt(&ok)));
        if (!ok) return;
        height = std::max(1, std::min(4096, text.midRef(sepPos + 1).toInt(&ok)));
        if (!ok) return;
    }

    getScene()->setCEGUIDisplaySize(width, height);
}
