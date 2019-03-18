#ifndef PENBUTTON_H
#define PENBUTTON_H

#include "qpushbutton.h"
#include "qpen.h"

class PenButton : public QPushButton
{
    Q_OBJECT

public:

    PenButton(QWidget* parent = nullptr);

    const QPen& pen() const { return _pen; }
    void setPen(const QPen& newPen);

signals:

    void penChanged(const QPen& newPen);

protected slots:

    void onClick();

protected:

    QPen _pen;
};

#endif // PENBUTTON_H
