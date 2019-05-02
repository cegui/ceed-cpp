#include "src/cegui/CEGUISerializedWidget.h"

CEGUISerializedWidgetPtr CEGUISerializedWidget::serialize(CEGUI::Window* widget, bool recursive)
{
    CEGUISerializedWidgetPtr data(new CEGUISerializedWidget);
    if (!data->serializeImpl(widget, recursive)) data.reset();
    return data;
}

CEGUI::Window* CEGUISerializedWidget::deserialize() const
{
    return nullptr;
}

CEGUISerializedWidget::CEGUISerializedWidget()
{
}

bool CEGUISerializedWidget::serializeImpl(CEGUI::Window* widget, bool recursive)
{
    /*
        self.name = ""
        self.type = ""
        # full parent path at the time of serialisation
        self.parentPath = ""
        # if True, the widget was an auto-widget, therefore it will not be
        # created directly when reconstructing but instead just retrieved
        self.autoWidget = False
        self.properties = {}
        self.children = []

        if widget is not None:
            self.name = widget.getName()
            self.type = widget.getType()
            self.autoWidget = widget.isAutoWindow()

            parent = widget.getParent()
            if parent is not None:
                self.parentPath = parent.getNamePath()

            self.serialiseProperties(widget)

            if serialiseChildren:
                self.serialiseChildren(widget)
    */
    return false;
}

/*

class SerialisationData(cegui_widgethelpers.SerialisationData):
    """See cegui.widgethelpers.SerialisationData

    The only reason for this class is that we need to create the correct Manipulator (not it's base class!)
    """

    def __init__(self, visual, widget = None, serialiseChildren = True):
        self.visual = visual

        super(SerialisationData, self).__init__(widget, serialiseChildren)

    def createChildData(self, widget = None, serialiseChildren = True):
        return SerialisationData(self.visual, widget, serialiseChildren)

    def createManipulator(self, parentManipulator, widget, recursive = True, skipAutoWidgets = True):
        ret = Manipulator(self.visual, parentManipulator, widget, recursive, skipAutoWidgets)
        ret.updateFromWidget()
        return ret

    def setVisual(self, visual):
        self.visual = visual

        for child in self.children:
            child.setVisual(visual)

    def reconstruct(self, rootManipulator):
        ret = super(SerialisationData, self).reconstruct(rootManipulator)

        if ret.parentItem() is None:
            # this is a root widget being reconstructed, handle this accordingly
            self.visual.setRootWidgetManipulator(ret)

        return ret
*/

/*
    def setParentPath(self, parentPath):
        """Recursively changes the parent path
        """

        self.parentPath = parentPath

        for child in self.children:
            child.setParentPath(self.parentPath + "/" + self.name)

    def createChildData(self, widget, serialiseChildren = True):
        """Creates child serialisation data of this widget
        """

        return SerialisationData(widget, serialiseChildren)

    def createManipulator(self, parentManipulator, widget, recursive = True, skipAutoWidgets = False):
        """Creates a manipulator suitable for the widget resulting from reconstruction
        """

        ret = Manipulator(parentManipulator, widget, recursive, skipAutoWidgets)
        ret.updateFromWidget()
        return ret

        // From reconstruct:
                # Extra code because of auto windows
                # NOTE: We don't have to do rootManipulator.createChildManipulators
                #       because auto windows never get created outside their parent
                parentManipulator.createChildManipulators(True, False)

                realPathSplit = widget.getNamePath().split("/", 1)
                ret = parent.getChildByPath(realPathSplit[1])
*/
