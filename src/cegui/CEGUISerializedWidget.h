#ifndef CEGUISERIALIZEDWIDGET_H
#define CEGUISERIALIZEDWIDGET_H

//???????need class? or serialize to QByteArray?

#include <memory>

// Allows to "freeze" CEGUI widget to data. This is a helper class for copy/paste, undo commands etc.

namespace CEGUI
{
    class Window;
}

typedef std::unique_ptr<class CEGUISerializedWidget> CEGUISerializedWidgetPtr;

class CEGUISerializedWidget
{
public:

    static CEGUISerializedWidgetPtr serialize(CEGUI::Window* widget, bool recursive = true);

    CEGUI::Window* deserialize() const;

protected:

    CEGUISerializedWidget();

    bool serializeImpl(CEGUI::Window* widget, bool recursive);
};

#endif // CEGUISERIALIZEDWIDGET_H
