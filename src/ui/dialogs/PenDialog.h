#ifndef PENDIALOG_H
#define PENDIALOG_H

#include <QDialog>

namespace Ui {
class PenDialog;
}

class PenDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PenDialog(QWidget *parent = nullptr);
    ~PenDialog();

    void setPen(const QPen& newPen);
    QPen getPen() const;

private:

    Ui::PenDialog *ui;
};

#endif // PENDIALOG_H
