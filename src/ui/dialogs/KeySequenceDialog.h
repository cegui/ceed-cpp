#ifndef KEYSEQUENCEDIALOG_H
#define KEYSEQUENCEDIALOG_H

#include <QDialog>

namespace Ui {
class KeySequenceDialog;
}

class KeySequenceDialog : public QDialog
{
    Q_OBJECT

public:

    explicit KeySequenceDialog(QWidget *parent = nullptr);
    ~KeySequenceDialog();

    void setKeySequence(QKeySequence newSeq);
    QKeySequence keySequence() const { return _seq; }

private:

    virtual void keyPressEvent(QKeyEvent* event) override;

    Ui::KeySequenceDialog *ui;

    QKeySequence _seq;
};

#endif // KEYSEQUENCEDIALOG_H
