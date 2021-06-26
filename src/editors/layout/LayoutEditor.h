#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include "src/editors/MultiModeEditor.h"

// Binds all layout editing functionality together

class Settings;
class Application;
class LayoutVisualMode;
class LayoutCodeMode;

class LayoutEditor : public MultiModeEditor
{
public:

    static void createSettings(Settings& mgr);
    static void createActions(Application& app);
    static void createToolbar(Application& app);

    LayoutEditor(const QString& filePath);

    bool loadVisualFromString(const QString& rawData);

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

    LayoutVisualMode* getVisualMode() const { return visualMode; }

protected:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;

    virtual void getRawData(QByteArray& outRawData) override;

    LayoutVisualMode* visualMode = nullptr;
    LayoutCodeMode* codeMode = nullptr;
};

class LayoutEditorFactory : public EditorFactoryBase
{
public:

    static QString layoutFileTypesDescription();
    static QStringList layoutFileExtensions();

    virtual QString getFileTypesDescription() const override { return layoutFileTypesDescription(); }
    virtual QStringList getFileExtensions() const override { return layoutFileExtensions(); }
    virtual bool requiresProject() const override { return true; }
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // LAYOUTEDITOR_H
