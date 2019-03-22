#include "src/editors/CodeEditMode.h"

// TODO: Some highlighting and other aids

CodeEditMode::CodeEditMode()
{
    document()->setUndoRedoEnabled(false);
    connect(document(), &QTextDocument::contentsChange, this, &CodeEditMode::slot_contentsChange);
}

void CodeEditMode::activate()
{
/*
        super(CodeEditMode, self).activate()

        self.refreshFromVisual()
*/
}

bool CodeEditMode::deactivate()
{
/*
        changesAccepted = self.propagateToVisual()
        ret = changesAccepted

        if not changesAccepted:
            # the file contains more than just CR LF
            result = QMessageBox.question(self,
                                 "Parsing the Code changes failed!",
                                 "Parsing of the changes done in Code edit mode failed, the result couldn't be accepted.\n"
                                 "Press Cancel to stay in the Code edit mode to correct the mistake(s) or press Discard to "
                                 "discard the changes and go back to the previous state (before you entered the code edit mode).",
                                 QMessageBox.Cancel | QMessageBox.Discard, QMessageBox.Cancel)

            if result == QMessageBox.Cancel:
                # return False to indicate we don't want to switch out of this widget
                ret = False

            elif result == QMessageBox.Discard:
                # we return True, the visual element wasn't touched (the error is thrown before that)
                ret = True

        return ret and super(CodeEditMode, self).deactivate()
*/
    return false;
}

// Refreshes this Code editing mode with current native source code
void CodeEditMode::refreshFromVisual()
{
/*
        source = self.getNativeCode()
*/
    ignoreUndoCommands = true;
/*
        self.setPlainText(source)
*/
    ignoreUndoCommands = false;
}

void CodeEditMode::slot_contentsChange(int position, int charsRemoved, int charsAdded)
{
    if (!ignoreUndoCommands)
    {
        int totalChange = charsRemoved + charsAdded;
        /*
            cmd = CodeEditModeCommand(self, self.lastUndoText, self.toPlainText(), totalChange)
            self.tabbedEditor.undoStack.push(cmd)
        */
    }

    lastUndoText = document()->toPlainText();
}

/*
    def getNativeCode(self):
        """Returns native source code from your editor implementation."""

        raise NotImplementedError("Every CodeEditing derived class must implement CodeEditing.getRefreshedNativeSource")
        #return ""

    def propagateNativeCode(self, code):
        """Synchronizes your editor implementation with given native source code.

        Returns True if changes were accepted (the code was valid, etc...)
        Returns False if changes weren't accepted (invalid code most likely)
        """

        raise NotImplementedError("Every CodeEditing derived class must implement CodeEditing.propagateNativeSource")
        #return False

    def propagateToVisual(self):
        """Propagates source code from this Code editing mode to your editor implementation.

        Returns True if changes were accepted (the code was valid, etc...)
        Returns False if changes weren't accepted (invalid code most likely)"""

        source = self.document().toPlainText()

        # for some reason, Qt calls hideEvent even though the tab widget was never shown :-/
        # in this case the source will be empty and parsing it will fail
        if source == "":
            return True

        return self.propagateNativeCode(source)

class CodeEditModeCommand(commands.UndoCommand):
    """
    Undo command for code edit mode.

    TODO: Extremely memory hungry implementation for now, I have to figure out how to use my own
    QUndoStack with QTextDocument in the future to fix this.
    """

    def __init__(self, codeEditing, oldText, newText, totalChange):
        super(CodeEditModeCommand, self).__init__()

        self.codeEditing = codeEditing
        self.oldText = oldText
        self.newText = newText

        self.totalChange = totalChange

        self.dryRun = True

        self.refreshText()

    def refreshText(self):
        if self.totalChange == 1:
            self.setText("Code edit, changed 1 character")
        else:
            self.setText("Code edit, changed %i characters" % (self.totalChange))

    def id(self):
        return 1000 + 1

    def mergeWith(self, cmd):
        assert(self.codeEditing == cmd.codeEditing)

        # TODO: 10 chars for now for testing
        if self.totalChange + cmd.totalChange < 10:
            self.totalChange += cmd.totalChange
            self.newText = cmd.newText

            self.refreshText()

            return True

        return False

    def undo(self):
        super(CodeEditModeCommand, self).undo()

        self.codeEditing.ignoreUndoCommands = True
        self.codeEditing.setPlainText(self.oldText)
        self.codeEditing.ignoreUndoCommands = False

    def redo(self):
        if not self.dryRun:
            self.codeEditing.ignoreUndoCommands = True
            self.codeEditing.setPlainText(self.newText)
            self.codeEditing.ignoreUndoCommands = False

        self.dryRun = False

        super(CodeEditModeCommand, self).redo()
*/
