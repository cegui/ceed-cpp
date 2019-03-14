#ifndef CEGUIPROJECTMANAGER_H
#define CEGUIPROJECTMANAGER_H
#include "qstring.h"

// A singleton CEGUI project manager class controls the loaded project

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

    void createProject();
    void loadProject(const QString& fileName);
    void unloadProject();
    bool isProjectLoaded() const { return !loadedProjectFileName.isEmpty(); }

protected:

    QString loadedProjectFileName;
};

#endif // CEGUIPROJECTMANAGER_H
