#include "src/editors/TextEditor.h"
#include "qfile.h"

TextEditor::TextEditor(const QString& filePath)
    : EditorBase(/*nullptr,*/ filePath)
{
}

void TextEditor::initialize()
{
    EditorBase::initialize();

    textDocument = new QTextDocument;

    {
        QFile file(_filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            textDocument->setPlainText(file.readAll());
    }

    updateFont();

    widget.setDocument(textDocument);

    textDocument->setModified(false);
    textDocument->setUndoRedoEnabled(true);

    /*
        self.textDocument.undoAvailable.connect(self.slot_undoAvailable)
        self.textDocument.redoAvailable.connect(self.slot_redoAvailable)
    */
}

void TextEditor::finalize()
{
    widget.setDocument(nullptr);

    if (textDocument)
    {
        delete textDocument;
        textDocument = nullptr;
    }

    EditorBase::finalize();
}

void TextEditor::copy()
{
    widget.copy();
}

void TextEditor::cut()
{
    widget.cut();
}

void TextEditor::paste()
{
    widget.paste();
}

void TextEditor::deleteSelected()
{
    widget.textCursor().removeSelectedText();
}

void TextEditor::undo()
{
    if (textDocument) textDocument->undo();
}

void TextEditor::redo()
{
    if (textDocument) textDocument->redo();
}

void TextEditor::zoomIn()
{
    if (fontSize < 80)
    {
        ++fontSize;
        updateFont();
    }
}

void TextEditor::zoomOut()
{
    if (fontSize > 5)
    {
        --fontSize;
        updateFont();
    }
}

void TextEditor::zoomReset()
{
    if (fontSize != 10)
    {
        fontSize = 10;
        updateFont();
    }
}

bool TextEditor::hasChanges() const
{
    return textDocument && textDocument->isModified();
}

void TextEditor::updateFont()
{
    QFont font("Courier New", fontSize);
    font.setStyleHint(QFont::Monospace);
    textDocument->setDefaultFont(font);
}

/*
def slot_undoAvailable(self, available):
    self.mainWindow.undoAction.setEnabled(available)

def slot_redoAvailable(self, available):
    self.mainWindow.redoAction.setEnabled(available)

def saveAs(self, targetPath, updateCurrentPath = True):
    self.nativeData = self.textDocument.toPlainText()

    return super(TextTabbedEditor, self).saveAs(targetPath, updateCurrentPath)
*/

//---------------------------------------------------------------------

QString TextEditorFactory::getFileTypesDescription() const
{
    return "Text files";
}

QStringList TextEditorFactory::getFileExtensions() const
{
    return { "py", "lua", "txt", "xml", "ini", "log", "hrd",
        // FIXME: these are temporary until the appropriate editor is implemented
        "scheme", "font" };
}

EditorBasePtr TextEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<TextEditor>(filePath);
}
