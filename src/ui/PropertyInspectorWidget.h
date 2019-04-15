#ifndef PROPERTYINSPECTORWIDGET_H
#define PROPERTYINSPECTORWIDGET_H

#include <QWidget>

// TODO: try Q_PROPERTY for property editor?

class QLabel;

class PropertyInspectorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit PropertyInspectorWidget(QWidget *parent = nullptr);

    virtual QSize sizeHint() const override { return QSize(400, 600); } // We'd rather have this size

signals:

public slots:

    void filterChanged(const QString& filterText);

protected:

    virtual void resizeEvent(QResizeEvent* event) override;

    void updateSelectionLabelElidedText();

    QLabel* _selectionLabel = nullptr;
    QString _selectionObjectPath;
    QString _selectionObjectDescription;
    QString _selectionLabelTooltip;
};

#endif // PROPERTYINSPECTORWIDGET_H
