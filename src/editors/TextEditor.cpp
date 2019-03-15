#include "src/editors/TextEditor.h"

TextEditor::TextEditor(const QString& filePath)
    : EditorBase(/*nullptr,*/ filePath)
{
}

bool TextEditor::hasChanges() const
{
    return textDocument && textDocument->isModified();
}
