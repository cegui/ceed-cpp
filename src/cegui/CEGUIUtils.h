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

    QString getRelativePath(const CEGUI::Window* widget, const CEGUI::Window* parent);
    void removeNestedPaths(QStringList& paths);

    bool serializeWidget(const CEGUI::Window& widget, QDataStream& stream, bool recursive);
    CEGUI::Window* deserializeWidget(QDataStream& stream, CEGUI::Window* parent = nullptr, size_t index = std::numeric_limits<size_t>().max());

    void addChild(CEGUI::Window* parent, CEGUI::Window* widget);
    bool insertChild(CEGUI::Window* parent, CEGUI::Window* widget, size_t index);
    void removeChild(CEGUI::Window* widget);

    void setWidgetProperty(CEGUI::Window* widget, const CEGUI::String& name, const CEGUI::String& value);
    void setWidgetArea(CEGUI::Window* widget, const CEGUI::UVector2& pos, const CEGUI::USize& size);

    CEGUI::MouseButton qtMouseButtonToMouseButton(Qt::MouseButton button);
    CEGUI::Key::Scan qtKeyToKey(int key, bool numpad = false);
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
