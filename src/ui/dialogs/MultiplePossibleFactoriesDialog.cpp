#include "src/ui/dialogs/MultiplePossibleFactoriesDialog.h"
#include "ui_MultiplePossibleFactoriesDialog.h"
#include "src/editors/EditorBase.h"

MultiplePossibleFactoriesDialog::MultiplePossibleFactoriesDialog(const std::vector<EditorFactoryBase*>& variants, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiplePossibleFactoriesDialog)
{
    ui->setupUi(this);

    auto factoryChoice = findChild<QListWidget*>("factoryChoice");
    for (auto factory : variants)
    {
        auto item = new QListWidgetItem(factory->getFileTypesDescription()); //, factoryChoice);
        item->setData(Qt::UserRole, QVariant::fromValue(factory));
        factoryChoice->addItem(item);
    }
}

MultiplePossibleFactoriesDialog::~MultiplePossibleFactoriesDialog()
{
    delete ui;
}

EditorFactoryBase* MultiplePossibleFactoriesDialog::getSelectedFactory() const
{
    auto factoryChoice = findChild<QListWidget*>("factoryChoice");
    auto selection = factoryChoice->selectedItems();
    return (selection.size() == 1) ? selection[0]->data(Qt::UserRole).value<EditorFactoryBase*>() : nullptr;
}
