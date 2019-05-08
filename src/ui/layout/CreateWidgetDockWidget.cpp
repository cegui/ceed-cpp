#include "src/ui/layout/CreateWidgetDockWidget.h"
#include "src/cegui/CEGUIManager.h"
#include "ui_CreateWidgetDockWidget.h"

CreateWidgetDockWidget::CreateWidgetDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::CreateWidgetDockWidget)
{
    ui->setupUi(this);
}

CreateWidgetDockWidget::~CreateWidgetDockWidget()
{
    delete ui;
}

void CreateWidgetDockWidget::populate()
{
    ui->tree->clear();

    std::map<QString, QStringList> widgetsBySkin;
    CEGUIManager::Instance().getAvailableWidgetsBySkin(widgetsBySkin);

    for (auto& pair : widgetsBySkin)
    {
        QTreeWidgetItem* skinItem;
        if (pair.first == "__no_skin__")
        {
            skinItem = ui->tree->invisibleRootItem();
        }
        else
        {
            skinItem = new QTreeWidgetItem();
            skinItem->setText(0, pair.first);

            // This makes sure the skin item isn't draggable
            skinItem->setFlags(Qt::ItemIsEnabled);
            ui->tree->addTopLevelItem(skinItem);
        }

        // skinItem now represents the skin node, we add all widgets in that skin to it
        for (auto& widget : pair.second)
        {
            auto widgetItem = new QTreeWidgetItem();
            widgetItem->setText(0, widget);
            skinItem->addChild(widgetItem);
        }
    }
}
