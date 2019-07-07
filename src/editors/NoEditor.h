#ifndef NOEDITOR_H
#define NOEDITOR_H

#include "src/editors/EditorBase.h"
#include "qscrollarea.h"

// This is basically a stub editor, it simply displays a message
// and doesn't allow any sort of editing at all, all functionality is stubbed.
// This is for internal use only so there is no factory for this particular editor

class NoEditor final : public EditorBase
{
public:

    NoEditor(const QString& filePath, const QString& message);
    virtual ~NoEditor() override;

    virtual QWidget* getWidget() override { return widget; }

private:

    virtual QString getFileTypesDescription() const override { return QString(); }
    virtual QStringList getFileExtensions() const override { return QStringList(); }

    QScrollArea* widget = nullptr;
};

#endif // NOEDITOR_H
