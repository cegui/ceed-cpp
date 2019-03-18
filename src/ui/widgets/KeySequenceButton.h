#ifndef KEYSEQUENCEBUTTON_H
#define KEYSEQUENCEBUTTON_H

#include "qpushbutton.h"

class KeySequenceButton : public QPushButton
{
    Q_OBJECT

public:

    KeySequenceButton(QWidget* parent = nullptr);

    void setKeySequence(QKeySequence newSeq);
    QKeySequence keySequence() const { return _seq; }

signals:

    void keySequenceChanged(const QKeySequence& newSeq);

protected slots:

    void onClick();

protected:

    QKeySequence _seq;
};

#endif // KEYSEQUENCEBUTTON_H
