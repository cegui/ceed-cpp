#ifndef CEGUIManager_H
#define CEGUIManager_H
#include "qstring.h"
#include "qimage.h"
#include <memory>
#include <functional>

// A singleton CEGUI manager class controls the loaded project and encapsulates a running CEGUI instance.
// Right now CEGUI can only be instantiated once because it's full of singletons. This might change in the
// future though, then CEGUI instance may be allocated per project.
// TODO: support multiple projects (switchable on demand)

class CEGUIProject;
class QtnEnumInfo;
class QOpenGLContext;
class QOffscreenSurface;
class RedirectingCEGUILogger;
class CEGUIDebugInfo;

class CEGUIManager
{
public:

    static QString ceedProjectExtension() { return "ceed"; }
    static QString getEditorIDStringPrefix() { return "ceed_internal-"; }

    CEGUIManager();
    CEGUIManager(const CEGUIManager&) = delete;
    ~CEGUIManager();

    CEGUIManager& operator =(const CEGUIManager&) = delete;

    static CEGUIManager& Instance()
    {
        static CEGUIManager mgr;
        return mgr;
    }

    CEGUIProject* createProject(const QString& filePath, bool createResourceDirs);
    void loadProject(const QString& filePath);
    void unloadProject();
    bool isProjectLoaded() const { return currentProject != nullptr; }
    CEGUIProject* getCurrentProject() const { return currentProject.get(); }

    QStringList getAvailableSkins() const;
    QStringList getAvailableFonts() const;
    QStringList getAvailableImages() const;
    void getAvailableWidgetsBySkin(std::map<QString, QStringList>& out) const;
    QImage getWidgetPreviewImage(const QString& widgetType, int previewWidth = 128, int previewHeight = 64);

    bool syncProjectToCEGUIInstance();
    void ensureCEGUIInitialized();
    bool makeOpenGLContextCurrent();
    void doneOpenGLContextCurrent();
    void showDebugInfo();

    // Property framework support
    const QtnEnumInfo& enumHorizontalAlignment();
    const QtnEnumInfo& enumVerticalAlignment();
    const QtnEnumInfo& enumAspectMode();
    const QtnEnumInfo& enumDefaultParagraphDirection();
    const QtnEnumInfo& enumWindowUpdateMode();
    const QtnEnumInfo& enumHorizontalFormatting();
    const QtnEnumInfo& enumVerticalFormatting();
    const QtnEnumInfo& enumHorizontalTextFormatting();
    const QtnEnumInfo& enumVerticalTextFormatting();
    const QtnEnumInfo& enumAutoPositioning();

protected:

    void cleanCEGUIResources();

    QOpenGLContext* glContext = nullptr;
    QOffscreenSurface* surface = nullptr;
    RedirectingCEGUILogger* logger = nullptr;
    CEGUIDebugInfo* debugInfo = nullptr;

    QtnEnumInfo* _enumHorizontalAlignment = nullptr;
    QtnEnumInfo* _enumVerticalAlignment = nullptr;
    QtnEnumInfo* _enumAspectMode = nullptr;
    QtnEnumInfo* _enumDefaultParagraphDirection = nullptr;
    QtnEnumInfo* _enumWindowUpdateMode = nullptr;
    QtnEnumInfo* _enumHorizontalFormatting = nullptr;
    QtnEnumInfo* _enumVerticalFormatting = nullptr;
    QtnEnumInfo* _enumHorizontalTextFormatting = nullptr;
    QtnEnumInfo* _enumVerticalTextFormatting = nullptr;
    QtnEnumInfo* _enumAutoPositioning = nullptr;

    std::unique_ptr<CEGUIProject> currentProject;
    bool initialized = false;
    bool _isOpenGL3 = false;
};

#endif // CEGUIManager_H
