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
        ++it;

        if (widget.isPropertyBannedFromXML(propertyName)) continue;

        // FIXME IN CEGUI: LookNFeel can override property value, and if we don't save it we can lose it.
        // E.g. auto-surface is false by default but TaharezLook/FrameWindow sets it to true. So if we
        // don't save 'false' because it is default we will get 'true' from LnF after deserialization.
        //if (widget.isPropertyDefault(propertyName)) continue;

        stream << stringToQString(propertyName);
        stream << stringToQString(widget.getProperty(propertyName));
        ++propertyCount;
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
        if (parent && !insertChild(parent, widget, index))
        {
            CEGUI::WindowManager::getSingleton().destroyWindow(widget);
            return nullptr;
        }
    }

    qint16 propertyCount = 0;
    stream >> propertyCount;
    for (qint16 i = 0; i < propertyCount; ++i)
    {
        QString propertyName, propertyValue;
        stream >> propertyName;
        stream >> propertyValue;
        setWidgetProperty(widget, qStringToString(propertyName), qStringToString(propertyValue));
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

void setWidgetProperty(CEGUI::Window* widget, const CEGUI::String& name, const CEGUI::String& value)
{
    if (!widget) return;

    // Some properties require CEGUI OpenGL context to be active when being changed
    const bool oglContextDependent =
            // Imagery cache texture size may need to be changed
            (widget->isUsingAutoRenderingSurface() &&
                ((name == "Area") ||
                 (name == "Size") ||
                 (name == "MinSize") ||
                 (name == "MaxSize") ||
                 (name == "AdjustWidthToContent") ||
                 (name == "AdjustHeightToContent") ||
                 (name == "AspectRatio") ||
                 (name == "AspectMode"))) ||
            // Directly dependent on OpenGL
            (name == "AutoRenderingSurface") ||
            (name == "AutoRenderingSurfaceStencilEnabled");
    if (oglContextDependent) CEGUIManager::Instance().makeOpenGLContextCurrent();

    widget->setProperty(name, value);

    if (oglContextDependent) CEGUIManager::Instance().doneOpenGLContextCurrent();
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
CEGUI::Key::Scan qtKeyToKey(int key, bool numpad)
{
    // TODO: re-check sometimes!
    // CEGUI keys not supported by Qt:
    //    RightWindows    =0xDC,  /* Right Windows key - Correct spelling :) */
    //    RightShift      =0x36,
    //    Stop            =0x95,  /*(NEC PC98) */ - conflicts with WebStop now
    //    RightControl    =0x9D,
    //    Oem102          =0x56,  /* < > | on UK/Germany keyboards */
    //    Kana            =0x70,  /* (Japanese keyboard)            */
    //    AbntC1          =0x73,  /* / ? on Portugese (Brazilian) keyboards */
    //    Convert         =0x79,  /* (Japanese keyboard)            */
    //    NoConvert       =0x7B,  /* (Japanese keyboard)            */
    //    AbntC2          =0x7E,  /* Numpad . on Portugese (Brazilian) keyboards */
    //    Ax              =0x96,  /*                     (Japan AX) */
    //    Unlabeled       =0x97,  /*                        (J3100) */

    switch (key)
    {
        case Qt::Key_Escape: return CEGUI::Key::Scan::Esc;
        case Qt::Key_Tab: return CEGUI::Key::Scan::Tab;
        case Qt::Key_Backspace: return CEGUI::Key::Scan::Backspace;
        case Qt::Key_Return:
        case Qt::Key_Enter: return numpad ? CEGUI::Key::Scan::NumpadEnter : CEGUI::Key::Scan::Return;
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
        case Qt::Key_Comma: return numpad ? CEGUI::Key::Scan::NumpadComma : CEGUI::Key::Scan::Comma;
        case Qt::Key_Asterisk:
        case Qt::Key_multiply: return CEGUI::Key::Scan::Multiply; // 'Multiply' is a numpad '*' but we don't check flag, no other '+' exists
        case Qt::Key_Plus: return CEGUI::Key::Scan::Add; // 'Add' is a numpad '+' but we don't check flag, no other '+' exists
        case Qt::Key_Minus: return numpad ? CEGUI::Key::Scan::Subtract : CEGUI::Key::Scan::Minus;
        case Qt::Key_Period: return numpad ? CEGUI::Key::Scan::Decimal : CEGUI::Key::Scan::Period;
        case Qt::Key_Slash: return numpad ? CEGUI::Key::Scan::Divide : CEGUI::Key::Scan::ForwardSlash;
        case Qt::Key_Backslash: return CEGUI::Key::Scan::Backslash;
        case Qt::Key_0: return numpad ? CEGUI::Key::Scan::Numpad_0 : CEGUI::Key::Scan::Zero;
        case Qt::Key_1: return numpad ? CEGUI::Key::Scan::Numpad_1 : CEGUI::Key::Scan::One;
        case Qt::Key_2: return numpad ? CEGUI::Key::Scan::Numpad_2 : CEGUI::Key::Scan::Two;
        case Qt::Key_3: return numpad ? CEGUI::Key::Scan::Numpad_3 : CEGUI::Key::Scan::Three;
        case Qt::Key_4: return numpad ? CEGUI::Key::Scan::Numpad_4 : CEGUI::Key::Scan::Four;
        case Qt::Key_5: return numpad ? CEGUI::Key::Scan::Numpad_5 : CEGUI::Key::Scan::Five;
        case Qt::Key_6: return numpad ? CEGUI::Key::Scan::Numpad_6 : CEGUI::Key::Scan::Six;
        case Qt::Key_7: return numpad ? CEGUI::Key::Scan::Numpad_7 : CEGUI::Key::Scan::Seven;
        case Qt::Key_8: return numpad ? CEGUI::Key::Scan::Numpad_8 : CEGUI::Key::Scan::Eight;
        case Qt::Key_9: return numpad ? CEGUI::Key::Scan::Numpad_9 : CEGUI::Key::Scan::Nine;
        case Qt::Key_Colon: return CEGUI::Key::Scan::Colon;
        case Qt::Key_Semicolon: return CEGUI::Key::Scan::Semicolon;
        case Qt::Key_Equal: return numpad ? CEGUI::Key::Scan::NumpadEquals : CEGUI::Key::Scan::Equals;
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
        case Qt::Key_CapsLock: return CEGUI::Key::Scan::Capital;
        case Qt::Key_NumLock: return CEGUI::Key::Scan::NumLock;
        case Qt::Key_ScrollLock: return CEGUI::Key::Scan::ScrollLock;
        case Qt::Key_AltGr: return CEGUI::Key::Scan::RightAlt;
        case Qt::Key_Underscore: return CEGUI::Key::Scan::Underline;
        case Qt::Key_yen: return CEGUI::Key::Scan::Yen; // For Japanese keyboard
        case Qt::Key_Kanji: return CEGUI::Key::Scan::Kanji; // For Japanese keyboard
        case Qt::Key_Dead_Circumflex: return CEGUI::Key::Scan::PrevTrack; // For Japanese keyboard
        case Qt::Key_MediaTogglePlayPause: return CEGUI::Key::Scan::PlayPause;
        case Qt::Key_MediaPrevious: return CEGUI::Key::Scan::PrevTrack;
        case Qt::Key_MediaNext: return CEGUI::Key::Scan::NextTrack;
        case Qt::Key_MediaStop: return CEGUI::Key::Scan::MediaStop;
        case Qt::Key_VolumeUp: return CEGUI::Key::Scan::VolumeUp;
        case Qt::Key_VolumeDown: return CEGUI::Key::Scan::VolumeDown;
        case Qt::Key_VolumeMute: return CEGUI::Key::Scan::Mute;
        case Qt::Key_Select: return CEGUI::Key::Scan::MediaSelect;
        case Qt::Key_Favorites: return CEGUI::Key::Scan::WebFavorites;
        case Qt::Key_Search: return CEGUI::Key::Scan::WebSearch;
        case Qt::Key_Refresh: return CEGUI::Key::Scan::WebRefresh;
        case Qt::Key_Forward: return CEGUI::Key::Scan::WebForward;
        case Qt::Key_Back: return CEGUI::Key::Scan::WebBack;
        case Qt::Key_Stop: return CEGUI::Key::Scan::WebStop;
        case Qt::Key_HomePage: return CEGUI::Key::Scan::WebHome;
        //case Qt::Key_Stop: return CEGUI::Key::Scan::Stop; // NEC PC98
        case Qt::Key_PowerOff: return CEGUI::Key::Scan::Power;
        case Qt::Key_Sleep: return CEGUI::Key::Scan::Sleep;
        case Qt::Key_WakeUp: return CEGUI::Key::Scan::Wake;
        case Qt::Key_LaunchMail: return CEGUI::Key::Scan::Mail;
        case Qt::Key_Calculator: return CEGUI::Key::Scan::Calculator;
        case Qt::Key_Explorer: return CEGUI::Key::Scan::MyComputer; // OK for now
        case Qt::Key_ApplicationRight:
        case Qt::Key_ApplicationLeft: return CEGUI::Key::Scan::AppMenu;
        case Qt::Key_Dead_Grave: return CEGUI::Key::Scan::Grave;
        case Qt::Key_BracketLeft: return CEGUI::Key::Scan::LeftBracket;
        case Qt::Key_BracketRight: return CEGUI::Key::Scan::RightBracket;

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
