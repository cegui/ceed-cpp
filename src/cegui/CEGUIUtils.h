#ifndef CEGUIUTILS_H
#define CEGUIUTILS_H

#include "qstring.h"

namespace CEGUI
{
    class String;
    class Window;
}

namespace CEGUIUtils
{
    QString stringToQString(const CEGUI::String& str);
    CEGUI::String qStringToString(const QString& str);

    QString getValidWidgetName(const QString& name);

    QString getUniqueChildWidgetName(const CEGUI::Window& parent, const QString& baseName);
    CEGUI::String getUniqueChildWidgetName(const CEGUI::Window& parent, const CEGUI::String& baseName);

    bool serializeWidget(const CEGUI::Window& widget, QDataStream& stream, bool recursive);
    CEGUI::Window* deserializeWidget(QDataStream& stream, CEGUI::Window* parent = nullptr);
};

#endif // CEGUIUTILS_H
