#ifndef EDITORBASE_H
#define EDITORBASE_H

#include "qstring.h"

// This is the base class for a class that takes a file and allows manipulation
// with it. It occupies exactly 1 tab space.

class QWidget;

typedef std::unique_ptr<class EditorBase> EditorBasePtr;

class EditorBase
{
public:

    EditorBase();
    virtual ~EditorBase() {}

    virtual QWidget* getWidget() = 0;
    virtual bool hasChanges() const { return false; }
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return filePath; }

protected:

    QString filePath;
};

#endif // EDITORBASE_H
