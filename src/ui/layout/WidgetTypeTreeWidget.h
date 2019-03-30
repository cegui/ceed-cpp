#ifndef WIDGETTYPETREEWIDGET_H
#define WIDGETTYPETREEWIDGET_H

#include "qtreewidget.h"

// Represents a single available widget for creation (it has a mapping in the scheme or is
// a stock special widget - like DefaultWindow). Also provides previews for the widgets

class WidgetTypeTreeWidget : public QTreeWidget
{
public:

    WidgetTypeTreeWidget(QWidget* parent = nullptr);

protected:

    virtual void startDrag(Qt::DropActions supportedActions) override;
    virtual bool viewportEvent(QEvent* event) override;
};

#endif // WIDGETTYPETREEWIDGET_H
