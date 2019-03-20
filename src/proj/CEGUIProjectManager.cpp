#include "src/proj/CEGUIProjectManager.h"
#include "src/proj/CEGUIProject.h"
#include "src/Application.h"
#include "qmessagebox.h"
#include "qdir.h"

CEGUIProjectManager::CEGUIProjectManager()
{  
/*
    # we start CEGUI early and we always start it
    self.ceguiInstance = cegui.Instance()

    //!!!was created in a MainWindow!
    self.ceguiContainerWidget = cegui_container.ContainerWidget(self.ceguiInstance, self)
*/
}

CEGUIProject* CEGUIProjectManager::createProject(const QString& filePath, bool createResourceDirs)
{
    //???force unload prev project?
    assert(!isProjectLoaded());

    currentProject.reset(new CEGUIProject());
    currentProject->filePath = filePath;

    // Enforce the "project" extension
    if (!currentProject->filePath.endsWith(".project"))
        currentProject->filePath += ".project";

    if (createResourceDirs)
    {
        bool success = true;
        QDir prefix = QFileInfo(currentProject->filePath).dir();
        QStringList dirs = { "fonts", "imagesets", "looknfeel", "schemes", "layouts", "xml_schemas" };
        for (const auto& dirName : dirs)
        {
            QDir dir(prefix.filePath(dirName));
            if (!dir.exists())
            {
                if (!dir.mkdir("."))
                    success = false;
            }
        }

        if (!success)
        {
            QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                                  "Cannot create resource directories!",
                                  "There was a problem creating the resource directories. "
                                  "Do you have the proper permissions on the parent directory?");
        }
    }

    //???need?
    currentProject->save();
    loadProject(currentProject->filePath);

    return currentProject.get();
}

// Opens the project file given in 'path'. Assumes no project is opened at the point this is called.
// Caller must test if a project is opened and close it accordingly (with a dialog
// being shown if there are changes to it)
// Errors aren't indicated by exceptions or return values, dialogs are shown in case of errors.
void CEGUIProjectManager::loadProject(const QString& fileName)
{
    if (isProjectLoaded())
    {
        // TODO: error message
        assert(false);
        return;
    }

    currentProject.reset(new CEGUIProject());
    if (!currentProject->loadFromFile(fileName))
    {
        QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                              "Error when opening project",
                              QString("It seems project at path '%s' doesn't exist or you don't have rights to open it.").arg(fileName));
        currentProject.reset();
        return;
    }

    // NB: must not be called in createProject() for new projects because it will be called
    // after the initial project setup in a project settings dialog.
    syncProjectToCEGUIInstance();
}

void CEGUIProjectManager::saveProject(const QString& fileName)
{
    if (currentProject) currentProject->save(fileName);
}

//!!!separate UI, move it to MainWindow!
// Closes currently opened project. Assumes one is opened at the point this is called.
void CEGUIProjectManager::unloadProject()
{
    if (!currentProject) return;

    /*
        # clean resources that were potentially used with this project
        self.ceguiInstance.cleanCEGUIResources()
    */

    currentProject->unload();
    currentProject.reset();
}

// Synchronises current project to the CEGUI instance
bool CEGUIProjectManager::syncProjectToCEGUIInstance()
{
    if (!currentProject)
    {
        /*
            self.ceguiInstance.cleanCEGUIResources()
        */
        return true;
    }

    if (!currentProject->checkAllDirectories())
    {
        QMessageBox::warning(qobject_cast<Application*>(qApp)->getMainWindow(),
                             "At least one of project's resource directories is invalid",
                             "Project's resource directory paths didn't pass the sanity check, please check projects settings.");
        return false;
    }

/*
    try:
        self.ceguiInstance.syncToProject(self.project, self)

        return true;

    except Exception as e:
        QtGui.QMessageBox.warning(self, "Failed to synchronise embedded CEGUI to your project",
            "An attempt was made to load resources related to the project being opened, "
            "for some reason the loading didn't succeed so all resources were destroyed! "
            "The most likely reason is that the resource directories are wrong, this can "
            "be very easily remedied in the project settings.\n\n"
            "This means that editing capabilities of CEED will be limited to editing of files "
            "that don't require a project opened (for example: imagesets).")
*/

    return false;
}

/* Was not used in the original CEED:
 * //???do projectFilePath = newPath inside a project? Then this method is a wrapper and can be removed.
 *
    def saveProjectAs(self, newPath):
        """Saves currently opened project to a custom path. For best reliability, use absolute file path as newPath"""

        self.project.save(newPath)
        # set the project's file path to newPath so that if you press save next time it will save to the new path
        # (This is what is expected from applications in general I think)
        self.project.projectFilePath = newPath
*/
