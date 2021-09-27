#ifndef BITMAPEDITOR_H
#define BITMAPEDITOR_H

#include "src/editors/EditorBase.h"
#include "src/ui/widgets/BitmapEditorWidget.h"

// A simple external bitmap editor starter/image viewer

class BitmapEditor : public EditorBase
{
public:

    BitmapEditor(const QString& filePath);

    virtual void initialize() override;

    virtual QWidget* getWidget() override { return &widget; }

protected:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual QString getDefaultFolder(CEGUIProject* project) const override;

    BitmapEditorWidget widget;
};

class BitmapEditorFactory : public EditorFactoryBase
{
public:

    static QString bitmapFileTypesDescription();
    static QStringList bitmapFileExtensions();

    virtual QString getFileTypesDescription() const override { return bitmapFileTypesDescription(); }
    virtual QStringList getFileExtensions() const override { return bitmapFileExtensions(); }
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // BITMAPEDITOR_H
