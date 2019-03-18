#include "src/ui/widgets/KeySequenceButton.h"
#include "src/ui/dialogs/KeySequenceDialog.h"

KeySequenceButton::KeySequenceButton(QWidget* parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    setKeySequence(QKeySequence());

    connect(this, &QPushButton::clicked, this, &KeySequenceButton::onClick);
}

void KeySequenceButton::setKeySequence(QKeySequence newSeq)
{
    _seq = newSeq;
    setText(newSeq.toString());
    emit keySequenceChanged(_seq);
}

void KeySequenceButton::onClick()
{
    KeySequenceDialog dialog(this);
    dialog.setKeySequence(_seq);
    if (dialog.exec() == QDialog::Accepted)
        setKeySequence(dialog.keySequence());
}
