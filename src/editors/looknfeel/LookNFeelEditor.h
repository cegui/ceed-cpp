#ifndef LOOKNFEELEDITOR_H
#define LOOKNFEELEDITOR_H

#include "src/editors/MultiModeEditor.h"
#include <CEGUI/String.h>
#include <unordered_set>

// Binds all Look n' Feel editing functionality together

class Settings;
class LookNFeelVisualMode;
class LookNFeelCodeMode;

class LookNFeelEditor : public MultiModeEditor
{
public:

    static void createSettings(Settings& mgr);

    LookNFeelEditor(const QString& filePath);

    virtual void initialize() override;
    void mapAndLoadLookNFeelFileString(const QString& lookNFeelXML);
    void getWidgetLookFeelNames(std::unordered_set<CEGUI::String>& out) const;
    QString unmapWidgetLookReferences(const CEGUI::String& lookNFeelString);

    virtual QWidget* getWidget() override { return &tabs; }
    virtual bool requiresProject() const override { return true; }

    const QString& getTargetWidgetLook() const { return _targetWidgetLook; }

protected:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual QString getDefaultFolder(CEGUIProject* project) const override;

    virtual void getRawData(QByteArray& outRawData) override;

    LookNFeelVisualMode* visualMode = nullptr;
    LookNFeelCodeMode* codeMode = nullptr;

    std::map<CEGUI::String, CEGUI::String> _nameMappingsOfOwnedWidgetLooks;

    QString _targetWidgetLook; // The name of the widget we are targeting for editing
    QString _editorIDString;
};

class LookNFeelEditorFactory : public EditorFactoryBase
{
public:

    static QString lnfFileTypesDescription();
    static QStringList lnfFileExtensions();

    virtual QString getFileTypesDescription() const override { return lnfFileTypesDescription(); }
    virtual QStringList getFileExtensions() const override { return lnfFileExtensions(); }
    virtual bool requiresProject() const override { return true; }
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // LOOKNFEELEDITOR_H
