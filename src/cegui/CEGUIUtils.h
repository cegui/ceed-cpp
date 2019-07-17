#ifndef CEGUIUTILS_H
#define CEGUIUTILS_H

#include "qstring.h"
#include <CEGUI/InputEvent.h>

namespace CEGUI
{
    class String;
    class Window;
    class UVector3;
}

namespace CEGUIUtils
{
    QString stringToQString(const CEGUI::String& str);
    CEGUI::String qStringToString(const QString& str);

    QString getValidWidgetName(const QString& name);

    QString getUniqueChildWidgetName(const CEGUI::Window& parent, const QString& baseName);
    CEGUI::String getUniqueChildWidgetName(const CEGUI::Window& parent, const CEGUI::String& baseName);

    void removeNestedPaths(QStringList& paths);

    bool serializeWidget(const CEGUI::Window& widget, QDataStream& stream, bool recursive);
    CEGUI::Window* deserializeWidget(QDataStream& stream, CEGUI::Window* parent = nullptr);

    CEGUI::MouseButton qtMouseButtonToMouseButton(Qt::MouseButton button);
    CEGUI::Key::Scan qtKeyToKey(int key);
};

QDataStream& operator <<(QDataStream& stream, const CEGUI::UDim& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::UDim& value);
QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector2& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::UVector2& value);
QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector3& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::UVector3& value);
QDataStream& operator <<(QDataStream& stream, const CEGUI::USize& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::USize& value);
QDataStream& operator <<(QDataStream& stream, const CEGUI::URect& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::URect& value);
QDataStream& operator <<(QDataStream& stream, const CEGUI::UBox& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::UBox& value);

#endif // CEGUIUTILS_H
