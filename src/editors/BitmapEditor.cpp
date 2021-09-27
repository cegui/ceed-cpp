#include "src/editors/BitmapEditor.h"
#include <memory>

BitmapEditor::BitmapEditor(const QString& filePath)
    : EditorBase(/*nullptr,*/ filePath)
{
}

void BitmapEditor::initialize()
{
    EditorBase::initialize();
    widget.openFile(_filePath);
}

QString BitmapEditor::getFileTypesDescription() const
{
    return BitmapEditorFactory::bitmapFileTypesDescription();
}

QStringList BitmapEditor::getFileExtensions() const
{
    return BitmapEditorFactory::bitmapFileExtensions();
}

QString BitmapEditor::getDefaultFolder(CEGUIProject* /*project*/) const
{
    return "";
}

//---------------------------------------------------------------------

QString BitmapEditorFactory::bitmapFileTypesDescription()
{
    return "Bitmap";
}

QStringList BitmapEditorFactory::bitmapFileExtensions()
{
    return { "png", "jpg", "jpeg", "tga", "dds" };
}

EditorBasePtr BitmapEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<BitmapEditor>(filePath);
}
