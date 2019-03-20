#include "src/proj/CEGUIProjectManager.h"
#include "CEGUIProject.h"

CEGUIProjectManager::CEGUIProjectManager()
{

}

CEGUIProject* CEGUIProjectManager::createProject()
{
    //???unload prev project?

    currentProject.reset(new CEGUIProject());
/*
        ret.projectFilePath = self.projectFilePath.text()

        if not ret.projectFilePath.endswith(".project"):
            # enforce the "project" extension
            ret.projectFilePath += ".project"

        if self.createResourceDirs.checkState() == QtCore.Qt.Checked:
            try:
                prefix = os.path.dirname(ret.projectFilePath)
                dirs = ["fonts", "imagesets", "looknfeel", "schemes", "layouts", "xml_schemas"]

                for dir_ in dirs:
                    if not os.path.exists(os.path.join(prefix, dir_)):
                        os.mkdir(os.path.join(prefix, dir_))

            except OSError as e:
                QtGui.QMessageBox.critical(self, "Cannot create resource \
directories!", "There was a problem creating the resource \
directories.  Do you have the proper permissions on the \
parent directory? (exception info: %s)" % (e))
*/
    return currentProject.get();
}

// Opens the project file given in 'path'. Assumes no project is opened at the point this is called.
// Caller must test if a project is opened and close it accordingly (with a dialog
// being shown if there are changes to it)
// Errors aren't indicated by exceptions or return values, dialogs are shown in case of errors.
// path - Absolute path of the project file
// openSettings - if True, the settings dialog is opened instead of just loading the resources,
//               this is desirable when creating a new project
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
        /*
        QMessageBox::critical(this,
                              "Error when opening project",
                              tr("It seems project at path '%s' doesn't exist or you don't have rights to open it.").arg(fileName));
        */
        currentProject.reset();
        return;
    }

    //!!!FORMER performProjectDirectoriesSanityCheck!
    if (!currentProject->checkAllDirectories())
    {
        /*
        if (indicateErrorsWithDialogs)
            QMessageBox::warning(self,
                                 "At least one of project's resource directories is invalid",
                                 "Project's resource directory paths didn't pass the sanity check, please check projects settings.
                                 "Details of this error: %s");
        */
        return;
    }

    /*
        if openSettings:
            self.slot_projectSettings() //!!!external, in MainWindow!

        else:
        //!!!checkAllDirectories / performProjectDirectoriesSanityCheck is inside, don't call twice!
            self.syncProjectToCEGUIInstance()
    */

    //!!!see updateUIOnProjectChanged for UI changes!

    loadedProjectFileName = fileName;
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
        # since we are effectively unloading the project and potentially nuking resources of it
        # we should definitely unload all tabs that rely on it to prevent segfaults and other
        # nasty phenomena
        if not self.closeAllTabsRequiringProject():
            return

        self.projectManager.setProject(None)
        # clean resources that were potentially used with this project
        self.ceguiInstance.cleanCEGUIResources()

        self.project.unload()
    */

    loadedProjectFileName.clear();
    currentProject.reset();

/*
        # as the project was closed be will disable actions related to it
        self.saveProjectAction.setEnabled(False)
        self.closeProjectAction.setEnabled(False)
        self.projectReloadResourcesAction.setEnabled(False)
*/
}

/*

    def syncProjectToCEGUIInstance(self, indicateErrorsWithDialogs = True):
        """Synchronises current project to the CEGUI instance.

        //!!!already done in loadProject, but required in 2 other places!!!
        self.performProjectDirectoriesSanityCheck()

        indicateErrorsWithDialogs - if True a dialog is opened in case of errors

        Returns True if the procedure was successful
        """

        try:
            self.ceguiInstance.syncToProject(self.project, self)

            return True

        except Exception as e:
            if indicateErrorsWithDialogs:
                QtGui.QMessageBox.warning(self, "Failed to synchronise embedded CEGUI to your project",
"""An attempt was made to load resources related to the project being opened, for some reason the loading didn't succeed so all resources were destroyed! The most likely reason is that the resource directories are wrong, this can be very easily remedied in the project settings.

This means that editing capabilities of CEED will be limited to editing of files that don't require a project opened (for example: imagesets).

Details of this error: %s""" % (e))

            return False
*/

/* Was not used in the original CEED:
 *
    def saveProjectAs(self, newPath):
        """Saves currently opened project to a custom path. For best reliability, use absolute file path as newPath
        """

        self.project.save(newPath)
        # set the project's file path to newPath so that if you press save next time it will save to the new path
        # (This is what is expected from applications in general I think)
        self.project.projectFilePath = newPath
*/
