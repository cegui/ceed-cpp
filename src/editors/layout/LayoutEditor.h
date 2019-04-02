#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include "src/editors/MultiModeEditor.h"

// Binds all layout editing functionality together

class ActionManager;
class Settings;
class LayoutVisualMode;
class LayoutCodeMode;

class LayoutEditor : public MultiModeEditor
{
public:

    static void createActions(ActionManager& mgr);
    static void createSettings(Settings& mgr);

    LayoutEditor(const QString& filePath);

    virtual void initialize() override;
    virtual void activate(MainWindow& mainWindow) override;
    virtual void deactivate(MainWindow& mainWindow) override;

    // Application commands implementation
    virtual void copy() override;
    virtual void cut() override;
    virtual void paste() override;
    virtual void deleteSelected() override;
    virtual void zoomIn() override;
    virtual void zoomOut() override;
    virtual void zoomReset() override;
//    //virtual void zoomFit() {}

    virtual QWidget* getWidget() override { return &tabs; }
    //virtual bool hasChanges() const;
    virtual bool requiresProject() const override { return true; }

protected:

    virtual void getRawData(QByteArray& outRawData) override;
    //virtual void markAsUnchanged();

    LayoutVisualMode* visualMode = nullptr;
    LayoutCodeMode* codeMode = nullptr;
};

class LayoutEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // LAYOUTEDITOR_H
