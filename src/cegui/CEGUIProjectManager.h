#ifndef CEGUIPROJECTMANAGER_H
#define CEGUIPROJECTMANAGER_H
#include "qstring.h"
#include "qimage.h"
#include <memory>

#define CEED_OPENGL_LEGACY_RENDERER

// A singleton CEGUI manager class controls the loaded project and encapsulates a running CEGUI instance.
// Right now CEGUI can only be instantiated once because it's full of singletons. This might change in the
// future though, then CEGUI instance may be allocated per project.
// TODO: support multiple projects (switchable on demand)

//???rename to CEGUIManager?

class CEGUIProject;
class CEGUIWidget; // TODO: one CEGUI widget per editor instead of the global one?
class QOpenGLContext;
class QOffscreenSurface;

class CEGUIProjectManager
{
public:

    static QString getEditorIDStringPrefix() { return "ceed_internal-"; }

    CEGUIProjectManager();
    CEGUIProjectManager(const CEGUIProjectManager&) = delete;
    ~CEGUIProjectManager();

    CEGUIProjectManager& operator =(const CEGUIProjectManager&) = delete;

    static CEGUIProjectManager& Instance()
    {
        static CEGUIProjectManager mgr;
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

protected:

    void cleanCEGUIResources();

    QOpenGLContext* glContext = nullptr;
    QOffscreenSurface* surface = nullptr;

    std::unique_ptr<CEGUIProject> currentProject;
    bool initialized = false;
};

#endif // CEGUIPROJECTMANAGER_H
