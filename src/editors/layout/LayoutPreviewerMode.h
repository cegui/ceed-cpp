#ifndef LAYOUTPREVIEWERMODE_H
#define LAYOUTPREVIEWERMODE_H

#include "src/editors/MultiModeEditor.h"
#include <QWidget>

// Provides "Live Preview" which is basically interactive CEGUI rendering
// without any other outlines or what not over it

namespace CEGUI
{
    class Window;
}

class LayoutEditor;

class LayoutPreviewerMode : public QWidget, public IEditMode
{
    Q_OBJECT

public:

    explicit LayoutPreviewerMode(LayoutEditor& editor, QWidget *parent = nullptr);

    virtual void activate() override;
    virtual bool deactivate() override;

protected:

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    CEGUI::Window* rootWidget = nullptr;
};

#endif // LAYOUTPREVIEWERMODE_H
