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
    ~CEGUIWidget();

    void setScene(CEGUIGraphicsScene* scene);
    CEGUIGraphicsScene* getScene() const;
    CEGUIGraphicsView* getView() const;

    void activate(QWidget* newParent, CEGUIGraphicsScene* scene = nullptr);
    void deactivate(QWidget* oldParent);

    void setViewFeatures(bool wheelZoom = false, bool middleButtonScroll = false, bool continuousRendering = true);
    void setInputEnabled(bool enable);

private slots:

    void on_debugInfoButton_clicked();

    void on_resolutionBox_editTextChanged(const QString& arg1);

private:

    Ui::CEGUIWidget *ui;
};

#endif // CEGUIWIDGET_H
