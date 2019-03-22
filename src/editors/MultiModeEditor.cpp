#include "src/editors/MultiModeEditor.h"

MultiModeEditor::MultiModeEditor(/*compatibilityManager, */ const QString& filePath)
    : EditorBase(filePath, true)
{
    tabs.setTabPosition(QTabWidget::South);
    tabs.setTabShape(QTabWidget::Triangular);

    connect(&tabs, &QTabWidget::currentChanged, this, &MultiModeEditor::slot_currentChanged);
}

void MultiModeEditor::initialize()
{
    EditorBase::initialize();

    // Used as a previous index in slot_currentChanged
    currentTabIndex = tabs.currentIndex();
    assert(currentTabIndex >= 0);
}

void MultiModeEditor::slot_currentChanged(int newTabIndex)
{
    if (ignoreCurrentChanged) return;

    auto prevTab = tabs.widget(currentTabIndex);
    auto currTab = tabs.widget(newTabIndex);

    if (prevTab)
    {
        /*
            if not oldTab.deactivate():
                self.ignoreCurrentChanged = True
                self.setCurrentWidget(oldTab)
                self.ignoreCurrentChanged = False

                return
        */
    }

    /*
    if newTab:
        newTab.activate()
    */

    if (!ignoreCurrentChangedForUndo)
    {
        /*
            cmd = ModeSwitchCommand(self, self.currentTabIndex, newTabIndex)
            self.undoStack.push(cmd)
        */
    }

    currentTabIndex = newTabIndex;
}


/*

class ModeSwitchCommand(commands.UndoCommand):
    """Undo command that is pushed to the undo stack whenever user switches edit modes.

    Switching edit mode has to be an undoable command because the other commands might
    or might not make sense if user is not in the right mode.

    This has a drawback that switching to Live Preview (layout editing) and back is
    undoable even though you can't affect the document in any way whilst in Live Preview
    mode.
    """

    def __init__(self, tabbedEditor, oldTabIndex, newTabIndex):
        super(ModeSwitchCommand, self).__init__()

        self.tabbedEditor = tabbedEditor

        self.oldTabIndex = oldTabIndex
        self.newTabIndex = newTabIndex

        # we never every merge edit mode changes, no need to define this as refreshText
        self.setText("Change edit mode to '%s'" % self.tabbedEditor.tabText(newTabIndex))

    def undo(self):
        super(ModeSwitchCommand, self).undo()

        self.tabbedEditor.ignoreCurrentChangedForUndo = True
        self.tabbedEditor.setCurrentIndex(self.oldTabIndex)
        self.tabbedEditor.ignoreCurrentChangedForUndo = False

    def redo(self):
        # to avoid multiple event firing
        if self.tabbedEditor.currentIndex() != self.newTabIndex:
            self.tabbedEditor.ignoreCurrentChangedForUndo = True
            self.tabbedEditor.setCurrentIndex(self.newTabIndex)
            self.tabbedEditor.ignoreCurrentChangedForUndo = False

        super(ModeSwitchCommand, self).redo()
*/
