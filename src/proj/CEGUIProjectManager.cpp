#include "src/proj/CEGUIProjectManager.h"

CEGUIProjectManager::CEGUIProjectManager()
{

}

void CEGUIProjectManager::loadProject(const QString& fileName)
{
    assert(loadedProjectFileName.isEmpty());
    if (!loadedProjectFileName.isEmpty()) return;

    loadedProjectFileName = fileName;
}

void CEGUIProjectManager::unloadProject()
{
    loadedProjectFileName.clear();
}
