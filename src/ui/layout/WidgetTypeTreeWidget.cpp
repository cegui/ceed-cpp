#include "src/ui/layout/WidgetTypeTreeWidget.h"
#include "src/cegui/CEGUIManager.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qpainter.h"
#include "qdrag.h"
#include "qbuffer.h"

WidgetTypeTreeWidget::WidgetTypeTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
}

// Shamelessly stolen from CELE2 by Paul D Turner (GPLv3)
void WidgetTypeTreeWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    auto item = currentItem();
    QString widgetType = item->text(0);
    QString look = item->parent() ? item->parent()->text(0) : "";
    widgetType = look.isEmpty() ? widgetType : (look + "/" + widgetType);

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/x-ceed-widget-type", widgetType.toUtf8());

    QPixmap pixmap(75, 40);
    QPainter painter(&pixmap);
    painter.eraseRect(0, 0, 75, 40);
    painter.setBrush(Qt::DiagCrossPattern);
    painter.drawRect(0, 0, 74, 39);
    painter.end();

    QDrag drag(this);
    drag.setMimeData(mimeData);
    drag.setPixmap(pixmap);
    drag.setHotSpot(QPoint(0, 0));
    drag.exec(Qt::CopyAction);
}

bool WidgetTypeTreeWidget::viewportEvent(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto item = itemAt(static_cast<QHelpEvent*>(event)->pos());
        if (item && !item->childCount())
        {
            const QString skin = item->parent() ? item->parent()->text(0) : "__no_skin__";
            const QString widgetType = item->text(0);

            const bool hasSkin = (skin != "__no_skin__");

            const QString fullWidgetType = hasSkin ? (skin + "/" + widgetType) : widgetType;

            QString tooltipText;
            if (!hasSkin)
                tooltipText = "Unskinned widgetType";
            else if (widgetType == "TabButton")
                tooltipText = "Can't render a preview as this is an autowidget, it requires a parent to be rendered.";
            else
            {
                try
                {
                    QByteArray bytes;
                    QBuffer buffer(&bytes);
                    buffer.open(QIODevice::WriteOnly);
                    CEGUIManager::Instance().getWidgetPreviewImage(fullWidgetType).save(&buffer, "PNG");
                    buffer.close();

                    tooltipText = QString("<img src=\"data:image/png;base64,%1\" />").arg(QString(bytes.toBase64()));
                }
                catch (const std::exception& e)
                {
                    tooltipText = e.what();
                }
            }

            item->setToolTip(0, QString("<small>Drag to the layout to create!</small><br />%1").arg(tooltipText));
        }
    }

    return QTreeWidget::viewportEvent(event);
}
