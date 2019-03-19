#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "src/editors/EditorBase.h"
#include "qtextedit.h"

// Multi purpose text editor
// TODO: This could use QSyntaxHighlighter

class TextEditor : public EditorBase
{
public:

    TextEditor(const QString& filePath);
    //virtual ~TextEditor() override {}

    virtual void initialize(/*mainWindow*/) override;
    virtual void finalize() override;

    virtual QWidget* getWidget() override { return &widget; }
    virtual bool hasChanges() const override;

protected:

    QTextEdit widget; //???if it is a pointer, should TextEditor/EditorBase be a QObject? or delete manually?
    QTextDocument* textDocument = nullptr;
};

class TextEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // TEXTEDITOR_H
