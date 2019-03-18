#include "src/ui/dialogs/KeySequenceDialog.h"
#include "ui_KeySequenceDialog.h"
#include "qevent.h"

KeySequenceDialog::KeySequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeySequenceDialog)
{
    setFocusPolicy(Qt::StrongFocus);

    ui->setupUi(this);
}

KeySequenceDialog::~KeySequenceDialog()
{
    delete ui;
}

void KeySequenceDialog::setKeySequence(QKeySequence newSeq)
{
    _seq = newSeq;

    auto keyCombination = findChild<QLineEdit*>("keyCombination");
    keyCombination->setText(newSeq.toString());
}

void KeySequenceDialog::keyPressEvent(QKeyEvent* event)
{
    setKeySequence(event->modifiers() | event->key());
}
