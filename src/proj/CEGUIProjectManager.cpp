#include "src/proj/CEGUIProjectManager.h"

CEGUIProjectManager::CEGUIProjectManager()
{

}

void CEGUIProjectManager::loadProject(const QString& fileName)
{
    loadedProjectFileName = fileName;
}

void CEGUIProjectManager::unloadProject()
{
    loadedProjectFileName.clear();
}
