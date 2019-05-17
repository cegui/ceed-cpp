#include "src/editors/looknfeel/LookNFeelScene.h"

LookNFeelScene::LookNFeelScene()
{

}

/*

class EditingScene(cegui_widgethelpers.GraphicsScene):
    """This scene contains all the manipulators users want to interact it. You can visualise it as the
    visual editing centre screen where CEGUI is rendered.

    It renders CEGUI on it's background and outlines (via Manipulators) in front of it.
    """

    def __init__(self, visual):
        super(EditingScene, self).__init__(mainwindow.MainWindow.instance.ceguiInstance)

        self.visual = visual
        self.ignoreSelectionChanges = False
        self.selectionChanged.connect(self.slot_selectionChanged)

    def setCEGUIDisplaySize(self, width, height, lazyUpdate = True):
        # overridden to keep the manipulators in sync

        super(EditingScene, self).setCEGUIDisplaySize(width, height, lazyUpdate)

    def slot_selectionChanged(self):
        selection = self.selectedItems()

        sets = []
        for item in selection:
            wdt = None

            if isinstance(item, widgethelpers.Manipulator):
                wdt = item.widget

            elif isinstance(item, resizable.ResizingHandle):
                if isinstance(item.parentResizable, widgethelpers.Manipulator):
                    wdt = item.parentResizable.widget

            if wdt is not None and wdt not in sets:
                sets.append(wdt)

    def mouseReleaseEvent(self, event):
        super(EditingScene, self).mouseReleaseEvent(event)

        movedWidgetPaths = []
        movedOldPositions = {}
        movedNewPositions = {}

        resizedWidgetPaths = []
        resizedOldPositions = {}
        resizedOldSizes = {}
        resizedNewPositions = {}
        resizedNewSizes = {}

        # we have to "expand" the items, adding parents of resizing handles
        # instead of the handles themselves
        expandedSelectedItems = []
        for selectedItem in self.selectedItems():
            if isinstance(selectedItem, widgethelpers.Manipulator):
                expandedSelectedItems.append(selectedItem)
            elif isinstance(selectedItem, resizable.ResizingHandle):
                if isinstance(selectedItem.parentItem(), widgethelpers.Manipulator):
                    expandedSelectedItems.append(selectedItem.parentItem())

        for item in expandedSelectedItems:
            if isinstance(item, widgethelpers.Manipulator):
                if item.preMovePos is not None:
                    widgetPath = item.widget.getNamePath()
                    movedWidgetPaths.append(widgetPath)
                    movedOldPositions[widgetPath] = item.preMovePos
                    movedNewPositions[widgetPath] = item.widget.getPosition()

                    # it won't be needed anymore so we use this to mark we picked this item up
                    item.preMovePos = None

                if item.preResizePos is not None and item.preResizeSize is not None:
                    widgetPath = item.widget.getNamePath()
                    resizedWidgetPaths.append(widgetPath)
                    resizedOldPositions[widgetPath] = item.preResizePos
                    resizedOldSizes[widgetPath] = item.preResizeSize
                    resizedNewPositions[widgetPath] = item.widget.getPosition()
                    resizedNewSizes[widgetPath] = item.widget.getSize()

                    # it won't be needed anymore so we use this to mark we picked this item up
                    item.preResizePos = None
                    item.preResizeSize = None

        if len(movedWidgetPaths) > 0:
            cmd = undoable_commands.MoveCommand(self.visual, movedWidgetPaths, movedOldPositions, movedNewPositions)
            self.visual.tabbedEditor.undoStack.push(cmd)

        if len(resizedWidgetPaths) > 0:
            cmd = undoable_commands.ResizeCommand(self.visual, resizedWidgetPaths, resizedOldPositions, resizedOldSizes, resizedNewPositions, resizedNewSizes)
            self.visual.tabbedEditor.undoStack.push(cmd)

    def keyReleaseEvent(self, event):
        handled = False

        if event.key() == QtCore.Qt.Key_Delete:
            handled = self.deleteSelectedWidgets()

        if not handled:
            super(EditingScene, self).keyReleaseEvent(event)

        else:
            event.accept()
*/
