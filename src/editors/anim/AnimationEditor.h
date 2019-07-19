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

protected:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
};

class AnimationEditorFactory : public EditorFactoryBase
{
public:

    static QString animationFileTypesDescription();
    static QStringList animationFileExtensions();

    virtual QString getFileTypesDescription() const override { return animationFileTypesDescription(); }
    virtual QStringList getFileExtensions() const override { return animationFileExtensions(); }
    virtual bool requiresProject() const override { return true; }
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // ANIMATIONEDITOR_H
