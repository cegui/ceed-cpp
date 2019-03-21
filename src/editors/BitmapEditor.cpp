#include "src/editors/BitmapEditor.h"

BitmapEditor::BitmapEditor(const QString& filePath)
    : EditorBase(/*nullptr,*/ filePath)
{
}

void BitmapEditor::initialize()
{
    EditorBase::initialize();
    widget.openFile(_filePath);
}

//---------------------------------------------------------------------

QString BitmapEditorFactory::getFileTypesDescription() const
{
    return "Text files";
}

QStringList BitmapEditorFactory::getFileExtensions() const
{
    return { "png", "jpg", "jpeg", "tga", "dds" };
}

EditorBasePtr BitmapEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<BitmapEditor>(filePath);
}
