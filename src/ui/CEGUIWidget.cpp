#include "src/ui/CEGUIWidget.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h"
#include "ui_CEGUIWidget.h"
#include <qlineedit.h>
#include <qtimer.h>

CEGUIWidget::CEGUIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CEGUIWidget)
{
    ui->setupUi(this);

    ui->view->setBackgroundRole(QPalette::Dark);

    ui->lblCursorPosition->setText("");
    connect(ui->view, &CEGUIGraphicsView::cursorPositionChanged, [this](int x, int y)
    {
        QString text("Cursor: x %1, y %2");
        ui->lblCursorPosition->setText(text.arg(x).arg(y));
    });

    updateZoomText();
    connect(ui->view, &ResizableGraphicsView::zoomChanged, [this](qreal /*factor*/)
    {
        updateZoomText();
    });
    connect(ui->view, &ResizableGraphicsView::helpOverlayVisibilityChanged, [this](bool visible)
    {
        if (ui->btnHelp->isChecked() != visible)
            ui->btnHelp->setChecked(visible);
    });

    ui->resolutionBox->installEventFilter(this);
    connect(ui->resolutionBox->lineEdit(), &QLineEdit::editingFinished, this, &CEGUIWidget::onResolutionTextChanged);
    connect(ui->resolutionBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CEGUIWidget::onResolutionTextChanged);
}

CEGUIWidget::~CEGUIWidget()
{
    delete ui;
}

void CEGUIWidget::setScene(CEGUIGraphicsScene* scene)
{
    ui->view->setScene(scene);

    // Make sure the resolution is set right for the given scene
    onResolutionTextChanged();

    // And mark the view as dirty to force Qt to redraw it
    ui->view->update();
}

CEGUIGraphicsScene* CEGUIWidget::getScene() const
{
    return static_cast<CEGUIGraphicsScene*>(ui->view->scene());
}

CEGUIGraphicsView* CEGUIWidget::getView() const
{
    return ui->view;
}

// The CEGUI view class has several enable/disable features that are very hard to achieve using
// inheritance/composition so they are kept in the CEGUI view class and its base class.
// This method enables/disables various features, calling it with no parameters switches to default.
// wheelZoom - mouse wheel will zoom in and out
// middleButtonScroll - pressing and dragging with the middle button will cause panning/scrolling
// continuousRendering - CEGUI will render continuously (not just when you tell it to)
void CEGUIWidget::setViewFeatures(bool wheelZoom, bool middleButtonScroll, bool continuousRendering, bool help)
{
    // Always zoom to the original 100% when changing view features
    ui->view->zoomReset();
    ui->view->setWheelZoomEnabled(wheelZoom);
    ui->view->setHelpEnabled(help);
    ui->view->setMiddleButtonDragScrollEnabled(middleButtonScroll);
    ui->view->setContinuousRendering(continuousRendering);

    ui->btnHelp->setVisible(help);
}

void CEGUIWidget::setResolution(int width, int height)
{
    if (!width && !height)
    {
        ui->resolutionBox->setCurrentText("Project default");
    }
    else
    {
        const QString resolutionStr = QString("%1x%2").arg(width).arg(height);
        ui->resolutionBox->setCurrentText(resolutionStr);
    }
    onResolutionTextChanged();
}

// If you have already activated this container, you can call this to enable CEGUI input propagation
// (The CEGUI instance associated will get mouse and keyboard events if the widget has focus)
void CEGUIWidget::setInputEnabled(bool enable)
{
    if (ui->view)
        ui->view->injectInput(enable);
}

void CEGUIWidget::updateZoomText()
{
    // We use the fact that our view is never rotated or sheared, so diagonal contains the scale
    // TODO: use SRT decomposition for stability!
    //assert(ui->view->transform().type() == QTransform::TxScale);
    QString text("Zoom: %1%");
    ui->lblZoom->setText(text.arg(static_cast<int>(ui->view->transform().m11() * 100.0)));
}

void CEGUIWidget::on_debugInfoButton_clicked()
{
    CEGUIManager::Instance().showDebugInfo();
}

void CEGUIWidget::on_btnHelp_toggled(bool checked)
{
    getView()->setHelpVisible(checked);
    getView()->setFocus();
}

void CEGUIWidget::onResolutionTextChanged()
{
    int width = 0;
    int height = 0;

    auto scene = getScene();

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

        if (scene &&
            qFuzzyCompare(scene->getContextWidth(), static_cast<float>(width)) &&
            qFuzzyCompare(scene->getContextHeight(), static_cast<float>(height)))
        {
            // Nothing changed
            return;
        }

        const QString resolutionStr = QString("%1x%2").arg(width).arg(height);
        if (ui->resolutionBox->findText(resolutionStr) < 0)
        {
            ui->resolutionBox->insertItem(0, resolutionStr);
            ui->resolutionBox->setCurrentIndex(0);
            return;
        }
    }

    if (scene) scene->setCEGUIDisplaySize(static_cast<float>(width), static_cast<float>(height));
}

bool CEGUIWidget::eventFilter(QObject* obj, QEvent* ev)
{
    // It seems that sync selectAll() happens before setting a cursor when focused, so we have to delay it
    if (obj == ui->resolutionBox && ev->type() == QEvent::FocusIn)
        QTimer::singleShot(0, ui->resolutionBox->lineEdit(), &QLineEdit::selectAll);
    return false;
}
