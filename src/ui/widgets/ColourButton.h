#ifndef COLOURBUTTON_H
#define COLOURBUTTON_H

#include "qpushbutton.h"

class ColourButton : public QPushButton
{
    Q_OBJECT

public:

    ColourButton(QWidget* parent = nullptr);

    const QColor& colour() const { return _colour; }
    void setColour(const QColor& newColour);

signals:

    void colourChanged(const QColor& color);

protected slots:

    void onClick();

protected:

    QColor _colour;
};

#endif // COLOURBUTTON_H
