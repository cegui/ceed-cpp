#ifndef CEGUIPROJECTMANAGER_H
#define CEGUIPROJECTMANAGER_H
#include "qstring.h"

// A singleton CEGUI project manager class controls the loaded project
// TODO: support multiple projects

//???rename to CEGUIManager & incorporate CEGUII instance?

class CEGUIProject;

class CEGUIProjectManager
{
public:

    CEGUIProjectManager();
    CEGUIProjectManager(const CEGUIProjectManager&) = delete;

    CEGUIProjectManager& operator =(const CEGUIProjectManager&) = delete;

    static CEGUIProjectManager& Instance()
    {
        static CEGUIProjectManager mgr;
        return mgr;
    }

    CEGUIProject* createProject();
    void loadProject(const QString& fileName);
    void saveProject(const QString& fileName = QString());
    void unloadProject();
    bool isProjectLoaded() const { return currentProject != nullptr; }
    CEGUIProject* getCurrentProject() const { return currentProject.get(); }

    bool syncProjectToCEGUIInstance();

protected:

    std::unique_ptr<CEGUIProject> currentProject;
};

#endif // CEGUIPROJECTMANAGER_H
