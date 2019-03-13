#ifndef FILELINEEDIT_H
#define FILELINEEDIT_H

#include <QWidget>

namespace Ui {
class FileLineEdit;
}

class FileLineEdit : public QWidget
{
    Q_OBJECT

public:
    explicit FileLineEdit(QWidget *parent = nullptr);
    ~FileLineEdit();

private:
    Ui::FileLineEdit *ui;
};

#endif // FILELINEEDIT_H
