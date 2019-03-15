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

    EditorBase(/*compatibilityManager, */ const QString& filePath);
    virtual ~EditorBase() {}

    void initialize(/*mainWindow*/);
    void finalize();
    void reloadData();
    void destroy();

    bool save() { return saveAs(_filePath); }
    bool saveAs(const QString& targetPath, bool updateCurrentPath = true);

    virtual QWidget* getWidget() = 0;
    virtual bool hasChanges() const { return false; }
    virtual bool requiresProject() const { return false; }

    QString getFilePath() const { return _filePath; }
    QString getLabelText() const { return _labelText; }

protected:

    QString _filePath;
    QString _labelText;
    bool _initialized = false;
};

#endif // EDITORBASE_H
