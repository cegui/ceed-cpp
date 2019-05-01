#ifndef CEGUIUTILS_H
#define CEGUIUTILS_H

#include "qstring.h"

namespace CEGUI
{
    class String;
}

namespace CEGUIUtils
{
    QString stringToQString(const CEGUI::String& str);
    CEGUI::String qStringToString(const QString& str);
};

#endif // CEGUIUTILS_H
