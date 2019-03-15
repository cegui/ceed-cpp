#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "src/editors/EditorBase.h"
#include "qtextedit.h"

// Multi purpose text editor
// TODO: This could use QSyntaxHighlighter

class TextEditor : public EditorBase
{
public:

    TextEditor();
    //virtual ~TextEditor() override {}

    virtual QWidget* getWidget() override { return &tabWidget; }

protected:

    QTextEdit tabWidget; //???if it is a pointer, should TextEditor/EditorBase be a QObject? or delete manually?
    QTextDocument* textDocument = nullptr;
};

#endif // TEXTEDITOR_H
