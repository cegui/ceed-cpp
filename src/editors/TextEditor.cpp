#include "src/editors/TextEditor.h"

TextEditor::TextEditor(/*filePath*/)
// : EditorBase(nullptr, filePath)
{
}

bool TextEditor::hasChanges() const
{
    return textDocument && textDocument->isModified();
}
