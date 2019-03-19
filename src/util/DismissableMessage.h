#ifndef DISMISSABLEMESSAGE_H
#define DISMISSABLEMESSAGE_H

#include "qwidget.h"

// Provides messages that users can dismiss (choose to never show again)

namespace DismissableMessage
{
    void warning(QWidget* parent, const QString& title, const QString& message, QString token = QString());
};

#endif // DISMISSABLEMESSAGE_H
