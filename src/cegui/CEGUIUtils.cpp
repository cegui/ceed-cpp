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

bool serializeWidget(const CEGUI::Window& widget, QDataStream& stream, bool recursive)
{
    if (!stream.device()->isWritable()) return false;

    stream << stringToQString(widget.getName());
    stream << stringToQString(widget.getType());
    stream << widget.isAutoWindow();

    //???need? lots of duplicated data for children
    stream << (widget.getParent() ? stringToQString(widget.getParent()->getNamePath()) : "");

    const auto propertyCounterPosition = stream.device()->pos();

    uint16_t propertyCount = 0;
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
        for (size_t i = 0; i < widget.getChildCount(); ++i)
        {
            const CEGUI::Window* child = widget.getChildAtIdx(i);
            if (child) // && (!skipAutoWidgets || !child->isAutoWindow()))
                serializeWidget(*child, stream, true);
        }
    }

    return true;
}

CEGUI::Window* deserializeWidget(const QByteArray& data, CEGUI::Window* parent)
{
    QDataStream stream(data);

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
        widget = CEGUI::WindowManager::getSingleton().createWindow(qStringToString(type), qStringToString(name));

        if (parent)
        {
            CEGUI::Window* realParent = nullptr;
            /*
                parentPathSplit = self.parentPath.split("/", 1)
                assert(len(parentPathSplit) >= 1)

                if len(parentPathSplit) == 1:
                    realParent = parent
                else:
                    realParent = parent.getChildByPath(parentPathSplit[1])
            */
            if (realParent) realParent->addChild(widget);
            /*
                # Extra code because of auto windows
                # NOTE: We don't have to do rootManipulator.createMissingChildManipulators
                #       because auto windows never get created outside their parent
                parentManipulator.createMissingChildManipulators(True, False)

                realPathSplit = widget.getNamePath().split("/", 1)
                ret = parent.getChildByPath(realPathSplit[1])
            */
        }
    }

    /*
            for name, value in self.properties.iteritems():
                widget.setProperty(name, value)

            for child in self.children:
                childWidget = child.reconstruct(rootManipulator).widget
                if not child.autoWidget:
                    widget.addChild(childWidget)

            # refresh the manipulator using newly set properties
            ret.updateFromWidget(False, True)
            return ret
    */

    return nullptr;
}

}
