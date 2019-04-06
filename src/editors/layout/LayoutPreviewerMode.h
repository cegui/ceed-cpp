#ifndef LAYOUTPREVIEWERMODE_H
#define LAYOUTPREVIEWERMODE_H

#include "src/editors/MultiModeEditor.h"
#include <QWidget>

// Provides "Live Preview" which is basically interactive CEGUI rendering
// without any other outlines or what not over it

class LayoutPreviewerMode : public QWidget, public IEditMode
{
    Q_OBJECT

public:

    explicit LayoutPreviewerMode(MultiModeEditor& editor, QWidget *parent = nullptr);

    virtual void activate() override;
    virtual bool deactivate() override;

signals:

public slots:

protected:

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;
};

#endif // LAYOUTPREVIEWERMODE_H
