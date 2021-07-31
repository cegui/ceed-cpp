#include "src/cegui/CEGUIUtils.h"
#include "src/cegui/CEGUIManager.h" //!!!for OpenGL context! TODO: encapsulate?
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/CoordConverter.h>
#include <CEGUI/WindowManager.h>
#include <qdatastream.h>

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

void removeNestedPaths(QStringList& paths)
{
    paths.erase(std::remove_if(paths.begin(), paths.end(), [&](const QString& path)
    {
        for (const QString& potentialParentPath : paths)
            if (path.startsWith(potentialParentPath + '/'))
                return true;

        return false;
    }), paths.end());
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
            const CEGUI::Window* child = widget.getChildAtIndex(i);
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

CEGUI::Window* deserializeWidget(QDataStream& stream, CEGUI::Window* parent, size_t index)
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
        if (parent)
        {
            if (!insertChild(parent, widget, index)) return nullptr;
        }
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

void addChild(CEGUI::Window* parent, CEGUI::Window* widget)
{
    if (!parent || !widget) return;

    // Activate CEGUI OpenGL context for possible imagery cache FBO manipulations
    CEGUIManager::Instance().makeOpenGLContextCurrent();
    parent->addChild(widget);
    CEGUIManager::Instance().doneOpenGLContextCurrent();
}

void removeChild(CEGUI::Window* widget)
{
    if (!widget) return;

    if (auto parent = widget->getParent())
    {
        // Activate CEGUI OpenGL context for possible imagery cache FBO manipulations
        CEGUIManager::Instance().makeOpenGLContextCurrent();
        parent->removeChild(widget);
        CEGUIManager::Instance().doneOpenGLContextCurrent();
    }
}

bool insertChild(CEGUI::Window* parent, CEGUI::Window* widget, size_t index)
{
    if (!parent || !widget) return false;

    // Check grid layout capacity limitation
    if (auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(parent))
    {
        if (!glc->isAutoGrowing())
        {
            const size_t capacity = glc->getGridWidth() * glc->getGridHeight();
            if (capacity <= glc->getActualChildCount())
                return false;
        }
    }

    // Activate CEGUI OpenGL context for possible imagery cache FBO manipulations
    CEGUIManager::Instance().makeOpenGLContextCurrent();
    if (index < parent->getChildCount())
        parent->addChildAtIndex(widget, index);
    else
        parent->addChild(widget);
    CEGUIManager::Instance().doneOpenGLContextCurrent();

    return true;
}

CEGUI::MouseButton qtMouseButtonToMouseButton(Qt::MouseButton button)
{
    switch (button)
    {
        case Qt::RightButton: return CEGUI::MouseButton::Right;
        case Qt::MidButton: return CEGUI::MouseButton::Middle;
        case Qt::XButton1: return CEGUI::MouseButton::X1;
        case Qt::XButton2: return CEGUI::MouseButton::X2;
        default: return CEGUI::MouseButton::Left;
    }
}

// Shame this isn't standardised :-/ Was a pain to write down.
// And to port too...
CEGUI::Key::Scan qtKeyToKey(int key)
{
    // TODO: re-check, may have been added in Qt5
    // missing Backtab
    // missing Print
    // missing AltGr
    // missing CapsLock
    // missing NumLock
    // missing ScrollLock
    // missing F16 - F35
    // missing Exclam
    // missing Key_Less
    // missing Key_Greater
    // missing Key_Question
    // Qt::Key_Super_L    0x01000053
    // Qt::Key_Super_R    0x01000054
    // Qt::Key_Menu    0x01000055
    // Qt::Key_Hyper_L    0x01000056
    // Qt::Key_Hyper_R    0x01000057
    // Qt::Key_Help    0x01000058
    // Qt::Key_Direction_L    0x01000059
    // Qt::Key_Direction_R    0x01000060
    // Qt::Key_QuoteDbl    0x22
    // Qt::Key_NumberSign    0x23
    // Qt::Key_Dollar    0x24
    // Qt::Key_Percent    0x25
    // Qt::Key_Ampersand    0x26
    // Qt::Key_ParenLeft    0x28
    // Qt::Key_ParenRight    0x29
    // Qt::Key_Asterisk    0x2a
    // Qt::Key_Plus    0x2b

    switch (key)
    {
        case Qt::Key_Escape: return CEGUI::Key::Scan::Esc;
        case Qt::Key_Tab: return CEGUI::Key::Scan::Tab;
        case Qt::Key_Backspace: return CEGUI::Key::Scan::Backspace;
        case Qt::Key_Return:
        case Qt::Key_Enter: return CEGUI::Key::Scan::Return;
        case Qt::Key_Insert: return CEGUI::Key::Scan::Insert;
        case Qt::Key_Delete: return CEGUI::Key::Scan::DeleteKey;
        case Qt::Key_Pause: return CEGUI::Key::Scan::Pause;
        case Qt::Key_SysReq: return CEGUI::Key::Scan::SysRq;
        case Qt::Key_Home: return CEGUI::Key::Scan::Home;
        case Qt::Key_End: return CEGUI::Key::Scan::End;
        case Qt::Key_Left: return CEGUI::Key::Scan::ArrowLeft;
        case Qt::Key_Up: return CEGUI::Key::Scan::ArrowUp;
        case Qt::Key_Right: return CEGUI::Key::Scan::ArrowRight;
        case Qt::Key_Down: return CEGUI::Key::Scan::ArrowDown;
        case Qt::Key_PageUp: return CEGUI::Key::Scan::PageUp;
        case Qt::Key_PageDown: return CEGUI::Key::Scan::PageDown;
        case Qt::Key_Shift: return CEGUI::Key::Scan::LeftShift;
        case Qt::Key_Control: return CEGUI::Key::Scan::LeftControl;
        case Qt::Key_Meta: return CEGUI::Key::Scan::LeftWindows;
        case Qt::Key_Alt: return CEGUI::Key::Scan::LeftAlt;
        case Qt::Key_F1: return CEGUI::Key::Scan::F1;
        case Qt::Key_F2: return CEGUI::Key::Scan::F2;
        case Qt::Key_F3: return CEGUI::Key::Scan::F3;
        case Qt::Key_F4: return CEGUI::Key::Scan::F4;
        case Qt::Key_F5: return CEGUI::Key::Scan::F5;
        case Qt::Key_F6: return CEGUI::Key::Scan::F6;
        case Qt::Key_F7: return CEGUI::Key::Scan::F7;
        case Qt::Key_F8: return CEGUI::Key::Scan::F8;
        case Qt::Key_F9: return CEGUI::Key::Scan::F9;
        case Qt::Key_F10: return CEGUI::Key::Scan::F10;
        case Qt::Key_F11: return CEGUI::Key::Scan::F11;
        case Qt::Key_F12: return CEGUI::Key::Scan::F12;
        case Qt::Key_F13: return CEGUI::Key::Scan::F13;
        case Qt::Key_F14: return CEGUI::Key::Scan::F14;
        case Qt::Key_F15: return CEGUI::Key::Scan::F15;
        case Qt::Key_Space: return CEGUI::Key::Scan::Space;
        case Qt::Key_Apostrophe: return CEGUI::Key::Scan::Apostrophe;
        case Qt::Key_Comma: return CEGUI::Key::Scan::Comma;
        case Qt::Key_Minus: return CEGUI::Key::Scan::Minus;
        case Qt::Key_Period: return CEGUI::Key::Scan::Period;
        case Qt::Key_Slash: return CEGUI::Key::Scan::ForwardSlash;
        case Qt::Key_0: return CEGUI::Key::Scan::Zero;
        case Qt::Key_1: return CEGUI::Key::Scan::One;
        case Qt::Key_2: return CEGUI::Key::Scan::Two;
        case Qt::Key_3: return CEGUI::Key::Scan::Three;
        case Qt::Key_4: return CEGUI::Key::Scan::Four;
        case Qt::Key_5: return CEGUI::Key::Scan::Five;
        case Qt::Key_6: return CEGUI::Key::Scan::Six;
        case Qt::Key_7: return CEGUI::Key::Scan::Seven;
        case Qt::Key_8: return CEGUI::Key::Scan::Eight;
        case Qt::Key_9: return CEGUI::Key::Scan::Nine;
        case Qt::Key_Colon: return CEGUI::Key::Scan::Colon;
        case Qt::Key_Semicolon: return CEGUI::Key::Scan::Semicolon;
        case Qt::Key_Equal: return CEGUI::Key::Scan::Equals;
        case Qt::Key_At: return CEGUI::Key::Scan::At;
        case Qt::Key_A: return CEGUI::Key::Scan::A;
        case Qt::Key_B: return CEGUI::Key::Scan::B;
        case Qt::Key_C: return CEGUI::Key::Scan::C;
        case Qt::Key_D: return CEGUI::Key::Scan::D;
        case Qt::Key_E: return CEGUI::Key::Scan::E;
        case Qt::Key_F: return CEGUI::Key::Scan::F;
        case Qt::Key_G: return CEGUI::Key::Scan::G;
        case Qt::Key_H: return CEGUI::Key::Scan::H;
        case Qt::Key_I: return CEGUI::Key::Scan::I;
        case Qt::Key_J: return CEGUI::Key::Scan::J;
        case Qt::Key_K: return CEGUI::Key::Scan::K;
        case Qt::Key_L: return CEGUI::Key::Scan::L;
        case Qt::Key_M: return CEGUI::Key::Scan::M;
        case Qt::Key_N: return CEGUI::Key::Scan::N;
        case Qt::Key_O: return CEGUI::Key::Scan::O;
        case Qt::Key_P: return CEGUI::Key::Scan::P;
        case Qt::Key_Q: return CEGUI::Key::Scan::Q;
        case Qt::Key_R: return CEGUI::Key::Scan::R;
        case Qt::Key_S: return CEGUI::Key::Scan::S;
        case Qt::Key_T: return CEGUI::Key::Scan::T;
        case Qt::Key_U: return CEGUI::Key::Scan::U;
        case Qt::Key_V: return CEGUI::Key::Scan::V;
        case Qt::Key_W: return CEGUI::Key::Scan::W;
        case Qt::Key_X: return CEGUI::Key::Scan::X;
        case Qt::Key_Y: return CEGUI::Key::Scan::Y;
        case Qt::Key_Z: return CEGUI::Key::Scan::Z;

        // The rest are weird keys I refuse to type here
        default: return CEGUI::Key::Scan::Unknown;
    }
}

}

QDataStream& operator <<(QDataStream& stream, const CEGUI::UDim& value)
{
    stream << value.d_scale;
    stream << value.d_offset;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::UDim& value)
{
    stream >> value.d_scale;
    stream >> value.d_offset;
    return stream;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector2& value)
{
    stream << value.d_x;
    stream << value.d_y;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::UVector2& value)
{
    stream >> value.d_x;
    stream >> value.d_y;
    return stream;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector3& value)
{
    stream << value.d_x;
    stream << value.d_y;
    stream << value.d_z;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::UVector3& value)
{
    stream >> value.d_x;
    stream >> value.d_y;
    stream >> value.d_z;
    return stream;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::USize& value)
{
    stream << value.d_width;
    stream << value.d_height;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::USize& value)
{
    stream >> value.d_width;
    stream >> value.d_height;
    return stream;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::URect& value)
{
    stream << value.d_min;
    stream << value.d_max;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::URect& value)
{
    stream >> value.d_min;
    stream >> value.d_max;
    return stream;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::UBox& value)
{
    stream << value.d_top;
    stream << value.d_left;
    stream << value.d_right;
    stream << value.d_bottom;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::UBox& value)
{
    stream >> value.d_top;
    stream >> value.d_left;
    stream >> value.d_right;
    stream >> value.d_bottom;
    return stream;
}
