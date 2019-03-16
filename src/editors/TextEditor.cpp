#include "src/editors/TextEditor.h"
#include "qfile.h"
#include "qfileinfo.h"

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

    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    textDocument->setDefaultFont(font);

    widget.setDocument(textDocument);

    textDocument->setModified(false);
    textDocument->setUndoRedoEnabled(true);

    /*
        self.textDocument.undoAvailable.connect(self.slot_undoAvailable)
        self.textDocument.redoAvailable.connect(self.slot_redoAvailable)
        self.textDocument.contentsChanged.connect(self.slot_contentsChanged)
    */
}

void TextEditor::finalize()
{
    if (textDocument)
    {
        delete textDocument;
        textDocument = nullptr;
    }

    EditorBase::finalize();
}

bool TextEditor::hasChanges() const
{
    return textDocument && textDocument->isModified();
}

/*
def undo(self):
    self.textDocument.undo()

def redo(self):
    self.textDocument.redo()

def slot_undoAvailable(self, available):
    self.mainWindow.undoAction.setEnabled(available)

def slot_redoAvailable(self, available):
    self.mainWindow.redoAction.setEnabled(available)

def slot_contentsChanged(self):
    self.markHasChanges(self.hasChanges())

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
    return { "py", "lua", "txt", "xml", "ini",
        // FIXME: these are temporary until the appropriate editor is implemented
        "scheme", "font" };
}

bool TextEditorFactory::canEditFile(const QString& filePath) const
{
    return getFileExtensions().contains(QFileInfo(filePath).completeSuffix());
}

EditorBasePtr TextEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<TextEditor>(filePath);
}
