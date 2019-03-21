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

    virtual void initialize() override;
    virtual void finalize() override;

    virtual void copy() override;
    virtual void cut() override;
    virtual void paste() override;
    virtual void deleteSelected() override;
    virtual void undo() override;
    virtual void redo() override;
    virtual void zoomIn() override;
    virtual void zoomOut() override;
    virtual void zoomReset() override;
    //virtual void zoomFit() {}

    virtual QWidget* getWidget() override { return &widget; }
    virtual bool hasChanges() const override;

protected:

    virtual void getRawData(QByteArray& outRawData) override;

    void updateFont();

    QTextEdit widget;
    QTextDocument* textDocument = nullptr;
    int fontSize = 10;
};

class TextEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // TEXTEDITOR_H
