#ifndef CEGUIWIDGET_H
#define CEGUIWIDGET_H

#include <QWidget>

// CEGUI context container widget with size control UI.
// This widget is what you should use (alongside your GraphicsScene derived class) to put CEGUI
// inside parts of the editor. Provides resolution changes, auto expanding and debug widget.

namespace Ui {
class CEGUIWidget;
}

class CEGUIGraphicsScene;
class CEGUIGraphicsView;

class CEGUIWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CEGUIWidget(QWidget* parent = nullptr);
    virtual ~CEGUIWidget() override;

    void setScene(CEGUIGraphicsScene* scene);
    CEGUIGraphicsScene* getScene() const;
    CEGUIGraphicsView* getView() const;

    void setViewFeatures(bool wheelZoom, bool middleButtonScroll, bool continuousRendering, bool help);
    void setResolution(int width, int height);
    void setInputEnabled(bool enable);
    void updateZoomText();

protected slots:

    void on_debugInfoButton_clicked();
    void on_btnHelp_toggled(bool checked);

    void onResolutionTextChanged();

protected:

    virtual bool eventFilter(QObject* obj, QEvent* ev) override;

    Ui::CEGUIWidget *ui;
};

#endif // CEGUIWIDGET_H
