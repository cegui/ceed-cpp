#include "src/editors/CodeEditMode.h"
#include "qmessagebox.h"
#include "qscrollbar.h"
#include <qevent.h>

// TODO: Some highlighting and other aids

CodeEditMode::CodeEditMode(MultiModeEditor& editor)
    : IEditMode(editor)
{
    document()->setUndoRedoEnabled(false);
    connect(document(), &QTextDocument::contentsChange, this, &CodeEditMode::slot_contentsChange);

    // TODO: unify with TextEditor?
    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    document()->setDefaultFont(font);
}

void CodeEditMode::keyPressEvent(QKeyEvent* event)
{
    if (event == QKeySequence::Undo)
    {
        _editor.getUndoStack()->undo();
        event->accept();
        return;
    }
    else if (event == QKeySequence::Redo)
    {
        _editor.getUndoStack()->redo();
        event->accept();
        return;
    }

    QTextEdit::keyPressEvent(event);
}

void CodeEditMode::activate(MainWindow& /*mainWindow*/, bool editorActivated)
{
    //IEditMode::activate(mainWindow);

    // Update text only when first opened or when an editor mode is switched
    if (!editorActivated) refreshFromVisual();
}

bool CodeEditMode::deactivate(MainWindow& /*mainWindow*/, bool editorDeactivated)
{
    if (editorDeactivated) return true;

    bool ret = propagateToVisual();
    if (!ret)
    {
        // The file contains more than just CR LF
        auto result = QMessageBox::question(this,
                             "Parsing the Code changes failed!",
                             "Parsing of the changes done in Code edit mode failed, the result couldn't be accepted.\n"
                             "Press Cancel to stay in the Code edit mode to correct the mistake(s) or press Discard to "
                             "discard the changes and go back to the previous state (before you entered the code edit mode).",
                             QMessageBox::Cancel | QMessageBox::Discard,
                             QMessageBox::Cancel);

        if (result == QMessageBox::Cancel)
            ret = false; // We don't want to switch out of this widget
        else if (result == QMessageBox::Discard)
            ret = true; // The visual element wasn't touched (the error is thrown before that)
    }

    return ret; // && IEditMode::deactivate(mainWindow);
}

void CodeEditMode::refreshFromVisual()
{
    setCodeWithoutUndoHistory(getNativeCode());
}

// Propagates source code from this Code editing mode to your editor implementation.
// Returns true if changes were accepted (code if valid) & false otherwise.
bool CodeEditMode::propagateToVisual()
{
    auto source = document()->toPlainText();

    // For some reason, Qt calls hideEvent even though the tab widget was never shown :-/
    // in this case the source will be empty and parsing it will fail
    if (source.isEmpty()) return true;

    return propagateNativeCode(source);
}

void CodeEditMode::setCodeWithoutUndoHistory(const QString& code)
{
    ignoreUndoCommands = true;
    setPlainText(code);
    ignoreUndoCommands = false;
}

void CodeEditMode::slot_contentsChange(int /*position*/, int charsRemoved, int charsAdded)
{
    if (!ignoreUndoCommands)
    {
        int totalChange = charsRemoved + charsAdded;
        _editor.getUndoStack()->push(new CodeEditModeCommand(*this, lastUndoText, toPlainText(), totalChange));
    }

    lastUndoText = document()->toPlainText();
}

//---------------------------------------------------------------------

// Refreshes this Code editing mode with current native source code and moves to the last scroll and cursor positions
ViewRestoringCodeEditMode::ViewRestoringCodeEditMode(MultiModeEditor& editor)
    : CodeEditMode(editor)
{
}

void ViewRestoringCodeEditMode::refreshFromVisual()
{
    CodeEditMode::refreshFromVisual();

    auto vbar = verticalScrollBar();
    vbar->setValue(lastVertScrollBarValue);

    auto cur = textCursor();
    cur.setPosition(lastCursorSelectionStart);
    cur.setPosition(lastCursorSelectionEnd, QTextCursor::KeepAnchor);
    setFocus();
    setTextCursor(cur);
}

// Propagates source code from this Code editing mode to your editor implementation and stores
// the last scrollbar and cursor positions
bool ViewRestoringCodeEditMode::propagateToVisual()
{
    auto vbar = verticalScrollBar();
    lastVertScrollBarValue = vbar->value();

    auto cur = textCursor();
    lastCursorSelectionStart = cur.selectionStart();
    lastCursorSelectionEnd = cur.selectionEnd();

    return CodeEditMode::propagateToVisual();
}

//---------------------------------------------------------------------

CodeEditModeCommand::CodeEditModeCommand(CodeEditMode& owner, const QString& oldText, const QString& newText, int totalChange)
    : _owner(owner)
    , _oldText(oldText)
    , _newText(newText)
    , _totalChange(totalChange)
{
    refreshText();
}

void CodeEditModeCommand::undo()
{
    QUndoCommand::undo();
    _owner.setCodeWithoutUndoHistory(_oldText);
}

void CodeEditModeCommand::redo()
{
    if (!_dryRun)
        _owner.setCodeWithoutUndoHistory(_newText);

    _dryRun = false;

    QUndoCommand::redo();
}

int CodeEditModeCommand::id() const
{
    return 1000 + 1;
}

// FIXME: need compact undo/redo, not the whole text saving. See QTextEdit source code. For 1.x release?
bool CodeEditModeCommand::mergeWith(const QUndoCommand* other)
{
    const CodeEditModeCommand* otherCmd = dynamic_cast<const CodeEditModeCommand*>(other);
    assert(&_owner == &otherCmd->_owner);

    // Slice changes by 64 chars for now, can change
    if (_totalChange + otherCmd->_totalChange < 64)
    {
        _totalChange += otherCmd->_totalChange;
        _newText = otherCmd->_newText;

        refreshText();

        return true;
    }

    return false;
}

void CodeEditModeCommand::refreshText()
{
    if (_totalChange == 1)
        setText("Code edit, changed 1 character");
    else
        setText(QString("Code edit, changed %1 characters").arg(_totalChange));
}
