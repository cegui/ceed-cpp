#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include "qdatastream.h"

namespace CEGUIUtils
{

QString stringToQString(const CEGUI::String& str)
{
#if (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_8 || CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_ASCII)
    return QString(str.c_str());
#elif (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_32)
    return QString(CEGUI::String::convertUtf32ToUtf8(str.c_str()).c_str());
#else
    #error "Unknown CEGUI::String implementation, consider adding support for it!"
#endif
}

CEGUI::String qStringToString(const QString& str)
{
    return CEGUI::String(str.toLocal8Bit().data());
}

// Returns a valid CEGUI widget name out of the supplied name, if possible. Returns empty string if
// the supplied name is invalid and can't be converted to a valid name (an empty string for example).
QString getValidWidgetName(const QString& name)
{
    return name.trimmed().replace("/", "_");
}

QString getUniqueChildWidgetName(const CEGUI::Window& parent, const QString& baseName)
{
    return stringToQString(getUniqueChildWidgetName(parent, qStringToString(baseName)));
}

// Finds a unique name for a child widget of the manipulated widget.
// The resulting name's format is the base with a number appended.
CEGUI::String getUniqueChildWidgetName(const CEGUI::Window& parent, const CEGUI::String& baseName)
{
    CEGUI::String candidate = baseName;
    int i = 2;
    while (parent.isChild(candidate))
    {
        candidate = baseName + std::to_string(i);
        ++i;
    }

    return candidate;
}

bool serializeWidget(const CEGUI::Window& widget, QDataStream& stream, bool recursive)
{
    if (!stream.device()->isWritable()) return false;

    stream << stringToQString(widget.getName());
    stream << stringToQString(widget.getType());
    stream << widget.isAutoWindow();

    const auto propertyCounterPosition = stream.device()->pos();

    qint16 propertyCount = 0;
    stream << propertyCount;

    auto it = widget.getPropertyIterator();
    while (!it.isAtEnd())
    {
        const auto& propertyName = it.getCurrentKey();
        if (!widget.isPropertyBannedFromXML(propertyName) && !widget.isPropertyDefault(propertyName))
        {
            stream << stringToQString(propertyName);
            stream << stringToQString(widget.getProperty(propertyName));
            ++propertyCount;
        }

        ++it;
    }

    const auto currPosition = stream.device()->pos();
    stream.device()->seek(propertyCounterPosition);
    stream << propertyCount;
    stream.device()->seek(currPosition);

    if (recursive)
    {
        assert(widget.getChildCount() < 65536);
        stream << static_cast<qint16>(widget.getChildCount());
        for (size_t i = 0; i < widget.getChildCount(); ++i)
        {
            const CEGUI::Window* child = widget.getChildAtIdx(i);
            if (child) // && (!skipAutoWidgets || !child->isAutoWindow()))
                serializeWidget(*child, stream, true);
        }
    }
    else
    {
        stream << static_cast<qint16>(0);
    }

    return true;
}

CEGUI::Window* deserializeWidget(QDataStream& stream, CEGUI::Window* parent)
{
    QString name, type;
    stream >> name;
    stream >> type;

    bool isAutoWidget = false;
    stream >> isAutoWidget;

    CEGUI::Window* widget = nullptr;

    if (isAutoWidget)
    {
        if (!parent)
        {
            assert(false && "Root widget can't be an auto widget!");
            return nullptr;
        }
        else
        {
            widget = parent->getChild(qStringToString(name));
            if (!widget || widget->getType() != qStringToString(type))
            {
                assert(false && "Skipping widget construction because it's an auto widget, the types don't match though!");
                return nullptr;
            }
        }
    }
    else
    {
        CEGUI::String widgetName = qStringToString(name);
        if (parent) widgetName = getUniqueChildWidgetName(*parent, widgetName);
        widget = CEGUI::WindowManager::getSingleton().createWindow(qStringToString(type), widgetName);
        if (parent) parent->addChild(widget);
    }

    qint16 propertyCount = 0;
    stream >> propertyCount;
    for (qint16 i = 0; i < propertyCount; ++i)
    {
        QString propertyName, propertyValue;
        stream >> propertyName;
        stream >> propertyValue;
        widget->setProperty(qStringToString(propertyName), qStringToString(propertyValue));
    }

    qint16 childCount = 0;
    stream >> childCount;
    for (qint16 i = 0; i < childCount; ++i)
        deserializeWidget(stream, widget);

    return widget;
}

}
