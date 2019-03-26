#ifndef LINEEDITWITHCLEARBUTTON_H
#define LINEEDITWITHCLEARBUTTON_H

#include "qlineedit.h"

// A QLineEdit with an inline clear button. Hitting Escape in the line edit clears it.
// Based on http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button/

class QToolButton;

class LineEditWithClearButton : public QLineEdit
{
public:

    LineEditWithClearButton(QWidget* parent = nullptr);

protected:

    virtual void resizeEvent(QResizeEvent* event) override;

    QToolButton* button = nullptr;
};

#endif // LINEEDITWITHCLEARBUTTON_H
