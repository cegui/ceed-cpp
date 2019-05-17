#ifndef LOOKNFEELEDITOR_H
#define LOOKNFEELEDITOR_H

#include "src/editors/MultiModeEditor.h"

// Binds all Look n' Feel editing functionality together

class LookNFeelEditor : public MultiModeEditor
{
public:

    static void createSettings(Settings& mgr);

    LookNFeelEditor(const QString& filePath);

    virtual void initialize() override;

    virtual QWidget* getWidget() override { return &tabs; }
    virtual bool requiresProject() const override { return true; }
};

class LookNFeelEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // LOOKNFEELEDITOR_H
