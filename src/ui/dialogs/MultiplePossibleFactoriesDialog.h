#ifndef MULTIPLEPOSSIBLEFACTORIESDIALOG_H
#define MULTIPLEPOSSIBLEFACTORIESDIALOG_H

#include <QDialog>

namespace Ui {
class MultiplePossibleFactoriesDialog;
}

class EditorFactoryBase;

class MultiplePossibleFactoriesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiplePossibleFactoriesDialog(const std::vector<EditorFactoryBase*>& variants, QWidget *parent = nullptr);
    ~MultiplePossibleFactoriesDialog();

    EditorFactoryBase* getSelectedFactory() const;

private:
    Ui::MultiplePossibleFactoriesDialog *ui;
};

#endif // MULTIPLEPOSSIBLEFACTORIESDIALOG_H
