#include "src/editors/TextEditor.h"
#include "qfile.h"

TextEditor::TextEditor(const QString& filePath)
    : EditorBase(/*nullptr,*/ filePath)
{
}

void TextEditor::initialize()
{
    EditorBase::initialize();

    textDocument = new QTextDocument();

    if (!_filePath.isEmpty())
    {
        QFile file(_filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            textDocument->setPlainText(file.readAll());
    }

    updateFont();

    widget.setDocument(textDocument);

    textDocument->setModified(false);
    textDocument->setUndoRedoEnabled(true);

    connect(textDocument, &QTextDocument::contentsChanged, [this]()
    {
        emit contentsChanged(textDocument->isModified());
    });
    connect(textDocument, &QTextDocument::undoAvailable, [this](bool available)
    {
        emit undoAvailable(available, "");
    });
    connect(textDocument, &QTextDocument::redoAvailable, [this](bool available)
    {
        emit redoAvailable(available, "");
    });
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

void TextEditor::getRawData(QByteArray& outRawData)
{
    if (textDocument)
        outRawData = textDocument->toPlainText().toUtf8();
}

void TextEditor::markAsUnchanged()
{
    if (textDocument) textDocument->setModified(false);
    EditorBase::markAsUnchanged();
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

QString TextEditor::getFileTypesDescription() const
{
    return TextEditorFactory::textFileTypesDescription();
}

QStringList TextEditor::getFileExtensions() const
{
    return TextEditorFactory::textFileExtensions();
}

void TextEditor::updateFont()
{
    QFont font("Courier New", fontSize);
    font.setStyleHint(QFont::Monospace);
    textDocument->setDefaultFont(font);
}

//---------------------------------------------------------------------

QString TextEditorFactory::textFileTypesDescription()
{
    return "Text";
}

QStringList TextEditorFactory::textFileExtensions()
{
    return { "txt", "xml", "py", "lua", "ini", "log", "hrd",
        // FIXME: these are temporary until the appropriate editor is implemented
        "scheme", "font" };
}

EditorBasePtr TextEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<TextEditor>(filePath);
}
