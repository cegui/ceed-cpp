#include "src/editors/layout/LayoutSerializationData.h"

LayoutSerializationData::LayoutSerializationData()
{

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
 *
 * // CEGUI (base) need 2 separate classes?

class SerialisationData(object):
    """Allows to "freeze" CEGUI widget to data that is easy to retain in python,
    this is a helper class that can be used for copy/paste, undo commands, etc...
    """

    def __init__(self, widget = None, serialiseChildren = True):
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

    def serialiseProperties(self, widget):
        """Takes a snapshot of all properties of given widget and stores them
        in a string form
        """

        it = widget.getPropertyIterator()
        while not it.isAtEnd():
            propertyName = it.getCurrentKey()

            if not widget.isPropertyBannedFromXML(propertyName) and not widget.isPropertyDefault(propertyName):
                self.properties[propertyName] = widget.getProperty(propertyName)

            it.next()

    def serialiseChildren(self, widget, skipAutoWidgets = False):
        """Serialises all child widgets of given widgets

        skipAutoWidgets - should auto widgets be skipped over?
        """

        i = 0

        while i < widget.getChildCount():
            child = widget.getChildAtIdx(i)
            if not skipAutoWidgets or not child.isAutoWindow():
                self.children.append(self.createChildData(child, True))

            i += 1

    def reconstruct(self, rootManipulator):
        """Reconstructs widget serialised in this SerialisationData.

        rootManipulator - manipulator at the root of the target tree
        """

        widget = None
        ret = None

        if rootManipulator is None:
            if self.autoWidget:
                raise RuntimeError("Root widget can't be an auto widget!")

            widget = PyCEGUI.WindowManager.getSingleton().createWindow(self.type, self.name)
            ret = self.createManipulator(None, widget)
            rootManipulator = ret

        else:
            parentManipulator = None
            parentPathSplit = self.parentPath.split("/", 1)
            assert(len(parentPathSplit) >= 1)

            if len(parentPathSplit) == 1:
                parentManipulator = rootManipulator
            else:
                parentManipulator = rootManipulator.getManipulatorByPath(parentPathSplit[1])

            widget = None
            if self.autoWidget:
                widget = parentManipulator.widget.getChild(self.name)
                if widget.getType() != self.type:
                    raise RuntimeError("Skipping widget construction because it's an auto widget, the types don't match though!")

            else:
                widget = PyCEGUI.WindowManager.getSingleton().createWindow(self.type, self.name)
                parentManipulator.widget.addChild(widget)

            # Extra code because of auto windows
            # NOTE: We don't have to do rootManipulator.createMissingChildManipulators
            #       because auto windows never get created outside their parent
            parentManipulator.createMissingChildManipulators(True, False)

            realPathSplit = widget.getNamePath().split("/", 1)
            ret = rootManipulator.getManipulatorByPath(realPathSplit[1])

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
