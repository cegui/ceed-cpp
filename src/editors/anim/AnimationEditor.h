#ifndef ANIMATIONEDITOR_H
#define ANIMATIONEDITOR_H

#include "src/editors/MultiModeEditor.h"

// Animation list file editor (XML file containing list of animations)

class AnimationEditor : public MultiModeEditor
{
public:

    AnimationEditor(const QString& filePath);

    virtual QWidget* getWidget() override { return &tabs; }
    virtual bool requiresProject() const override { return true; }
};

class AnimationEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // ANIMATIONEDITOR_H
