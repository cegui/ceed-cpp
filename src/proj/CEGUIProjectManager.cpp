#include "src/proj/CEGUIProjectManager.h"
#include "src/proj/CEGUIProject.h"
#include "src/Application.h"
#include "qmessagebox.h"
#include "qdir.h"

CEGUIProjectManager::CEGUIProjectManager()
{
/*
    self.logger = RedirectingCEGUILogger()
    self.lastRenderTimeDelta = 0

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
void CEGUIProjectManager::loadProject(const QString& filePath)
{
    if (isProjectLoaded())
    {
        // TODO: error message
        assert(false);
        return;
    }

    currentProject.reset(new CEGUIProject());
    if (!currentProject->loadFromFile(filePath))
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

// Ensures this CEGUI instance is properly initialised, if it's not it initialises it right away
void CEGUIProjectManager::ensureCEGUIInitialized()
{
    if (initialized) return;

    //???check FBO support here, when OpenGL is initialized? see MainWindow constructor

    // We don't want CEGUI Exceptions to output to stderr every time they are constructed
    /*
        PyCEGUI.Exception.setStdErrEnabled(False)
        # FBOs are for sure supported at this point because CEED uses them internally
        PyCEGUIOpenGLRenderer.OpenGLRenderer.bootstrapSystem(PyCEGUIOpenGLRenderer.OpenGLRenderer.TTT_FBO)

        // Put the resource groups to a reasonable default value, './datafiles' followed by
        // the respective folder, same as CEGUI stock datafiles
        defaultBaseDirectory = os.path.join(os.path.curdir, "datafiles")

        rp = PyCEGUI.System.getSingleton().getResourceProvider()
        rp.setResourceGroupDirectory("imagesets",
                                       os.path.join(defaultBaseDirectory, "imagesets"))
        rp.setResourceGroupDirectory("fonts",
                                       os.path.join(defaultBaseDirectory, "fonts"))
        rp.setResourceGroupDirectory("schemes",
                                       os.path.join(defaultBaseDirectory, "schemes"))
        rp.setResourceGroupDirectory("looknfeels",
                                       os.path.join(defaultBaseDirectory, "looknfeel"))
        rp.setResourceGroupDirectory("layouts",
                                       os.path.join(defaultBaseDirectory, "layouts"))
        rp.setResourceGroupDirectory("xml_schemas",
                                       os.path.join(defaultBaseDirectory, "xml_schemas"))

        // All this will never be set to anything else again
        PyCEGUI.ImageManager.setImagesetDefaultResourceGroup("imagesets")
        PyCEGUI.Font.setDefaultResourceGroup("fonts")
        PyCEGUI.Scheme.setDefaultResourceGroup("schemes")
        PyCEGUI.WidgetLookManager.setDefaultResourceGroup("looknfeels")
        PyCEGUI.WindowManager.setDefaultResourceGroup("layouts")

        parser = PyCEGUI.System.getSingleton().getXMLParser()
        if parser.isPropertyPresent("SchemaDefaultResourceGroup"):
            parser.setProperty("SchemaDefaultResourceGroup", "xml_schemas")
    */

    initialized = true;
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

    auto mainWnd = qobject_cast<Application*>(qApp)->getMainWindow();

    if (!currentProject->checkAllDirectories())
    {
        QMessageBox::warning(mainWnd,
                             "At least one of project's resource directories is invalid",
                             "Project's resource directory paths didn't pass the sanity check, please check projects settings.");
        return false;
    }

/*
    try:
        self.ceguiInstance.syncToProject(self.project, self)

//////////////
    def syncToProject(self, project, mainWindow = None):
        """Synchronises the instance with given project, respecting it's paths and resources
        """

        progress = QtGui.QProgressDialog(mainWindow)
        progress.setWindowModality(QtCore.Qt.WindowModal)
        progress.setWindowTitle("Synchronising embedded CEGUI with the project")
        progress.setCancelButton(None)
        progress.resize(400, 100)
        progress.show()

        self.ensureCEGUIInitialised()
        self.makeGLContextCurrent()

        schemeFiles = []
        absoluteSchemesPath = project.getAbsolutePathOf(project.schemesPath)
        if os.path.exists(absoluteSchemesPath):
            for file_ in os.listdir(absoluteSchemesPath):
                if file_.endswith(".scheme"):
                    schemeFiles.append(file_)
        else:
            progress.reset()
            raise IOError("Can't list scheme path '%s'" % (absoluteSchemesPath))

        progress.setMinimum(0)
        progress.setMaximum(2 + 9 * len(schemeFiles))

        progress.setLabelText("Purging all resources...")
        progress.setValue(0)
        QtGui.QApplication.instance().processEvents()

        # destroy all previous resources (if any)
        self.cleanCEGUIResources()

        progress.setLabelText("Setting resource paths...")
        progress.setValue(1)
        QtGui.QApplication.instance().processEvents()

        rp = PyCEGUI.System.getSingleton().getResourceProvider()
        rp.setResourceGroupDirectory("imagesets", project.getAbsolutePathOf(project.imagesetsPath))
        rp.setResourceGroupDirectory("fonts", project.getAbsolutePathOf(project.fontsPath))
        rp.setResourceGroupDirectory("schemes", project.getAbsolutePathOf(project.schemesPath))
        rp.setResourceGroupDirectory("looknfeels", project.getAbsolutePathOf(project.looknfeelsPath))
        rp.setResourceGroupDirectory("layouts", project.getAbsolutePathOf(project.layoutsPath))
        rp.setResourceGroupDirectory("xml_schemas", project.getAbsolutePathOf(project.xmlSchemasPath))

        progress.setLabelText("Recreating all schemes...")
        progress.setValue(2)
        QtGui.QApplication.instance().processEvents()

        # we will load resources manually to be able to use the compatibility layer machinery
        PyCEGUI.SchemeManager.getSingleton().setAutoLoadResources(False)

        from ceed import compatibility
        import ceed.compatibility.scheme as scheme_compatibility
        import ceed.compatibility.imageset as imageset_compatibility
        import ceed.compatibility.font as font_compatibility
        import ceed.compatibility.looknfeel as looknfeel_compatibility

        try:
            for schemeFile in schemeFiles:
                def updateProgress(message):
                    progress.setValue(progress.value() + 1)
                    progress.setLabelText("Recreating all schemes... (%s)\n\n%s" % (schemeFile, message))

                    QtGui.QApplication.instance().processEvents()

                updateProgress("Parsing the scheme file")
                schemeFilePath = project.getResourceFilePath(schemeFile, PyCEGUI.Scheme.getDefaultResourceGroup())
                rawData = open(schemeFilePath, "r").read()
                rawDataType = scheme_compatibility.manager.EditorNativeType

                try:
                    rawDataType = scheme_compatibility.manager.guessType(rawData, schemeFilePath)

                except compatibility.NoPossibleTypesError:
                    QtGui.QMessageBox.warning(None, "Scheme doesn't match any known data type", "The scheme '%s' wasn't recognised by CEED as any scheme data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!" % (schemeFilePath))
                    return

                except compatibility.MultiplePossibleTypesError as e:
                    suitableVersion = scheme_compatibility.manager.getSuitableDataTypeForCEGUIVersion(project.CEGUIVersion)

                    if suitableVersion not in e.possibleTypes:
                        QtGui.QMessageBox.warning(None, "Incorrect scheme data type", "The scheme '%s' checked out as some potential data types, however not any of these is suitable for your project's target CEGUI version '%s', please check your project settings! CEGUI instance synchronisation aborted!" % (schemeFilePath, suitableVersion))
                        return

                    rawDataType = suitableVersion

                nativeData = scheme_compatibility.manager.transform(rawDataType, scheme_compatibility.manager.EditorNativeType, rawData)
                scheme = PyCEGUI.SchemeManager.getSingleton().createFromString(nativeData)

                # NOTE: This is very CEGUI implementation specific unfortunately!
                #
                #       However I am not really sure how to do this any better.

                updateProgress("Loading XML imagesets")
                xmlImagesetIterator = scheme.getXMLImagesets()
                while not xmlImagesetIterator.isAtEnd():
                    loadableUIElement = xmlImagesetIterator.getCurrentValue()
                    imagesetFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else PyCEGUI.ImageManager.getImagesetDefaultResourceGroup())
                    imagesetRawData = open(imagesetFilePath, "r").read()
                    imagesetRawDataType = imageset_compatibility.manager.EditorNativeType

                    try:
                        imagesetRawDataType = imageset_compatibility.manager.guessType(imagesetRawData, imagesetFilePath)

                    except compatibility.NoPossibleTypesError:
                        QtGui.QMessageBox.warning(None, "Imageset doesn't match any known data type", "The imageset '%s' wasn't recognised by CEED as any imageset data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!" % (imagesetFilePath))
                        return

                    except compatibility.MultiplePossibleTypesError as e:
                        suitableVersion = imageset_compatibility.manager.getSuitableDataTypeForCEGUIVersion(project.CEGUIVersion)

                        if suitableVersion not in e.possibleTypes:
                            QtGui.QMessageBox.warning(None, "Incorrect imageset data type", "The imageset '%s' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%s', please check your project settings! CEGUI instance synchronisation aborted!" % (imagesetFilePath, suitableVersion))
                            return

                        imagesetRawDataType = suitableVersion

                    imagesetNativeData = imageset_compatibility.manager.transform(imagesetRawDataType, imageset_compatibility.manager.EditorNativeType, imagesetRawData)

                    PyCEGUI.ImageManager.getSingleton().loadImagesetFromString(imagesetNativeData)
                    xmlImagesetIterator.next()

                updateProgress("Loading image file imagesets")
                scheme.loadImageFileImagesets()

                updateProgress("Loading fonts")
                fontIterator = scheme.getFonts()
                while not fontIterator.isAtEnd():
                    loadableUIElement = fontIterator.getCurrentValue()
                    fontFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else PyCEGUI.Font.getDefaultResourceGroup())
                    fontRawData = open(fontFilePath, "r").read()
                    fontRawDataType = font_compatibility.manager.EditorNativeType

                    try:
                        fontRawDataType = font_compatibility.manager.guessType(fontRawData, fontFilePath)

                    except compatibility.NoPossibleTypesError:
                        QtGui.QMessageBox.warning(None, "Font doesn't match any known data type", "The font '%s' wasn't recognised by CEED as any font data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!" % (fontFilePath))
                        return

                    except compatibility.MultiplePossibleTypesError as e:
                        suitableVersion = font_compatibility.manager.getSuitableDataTypeForCEGUIVersion(project.CEGUIVersion)

                        if suitableVersion not in e.possibleTypes:
                            QtGui.QMessageBox.warning(None, "Incorrect font data type", "The font '%s' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%s', please check your project settings! CEGUI instance synchronisation aborted!" % (fontFilePath, suitableVersion))
                            return

                        fontRawDataType = suitableVersion

                    fontNativeData = font_compatibility.manager.transform(fontRawDataType, font_compatibility.manager.EditorNativeType, fontRawData)

                    PyCEGUI.FontManager.getSingleton().createFromString(fontNativeData)
                    fontIterator.next()

                updateProgress("Loading looknfeels")
                looknfeelIterator = scheme.getLookNFeels()
                while not looknfeelIterator.isAtEnd():
                    loadableUIElement = looknfeelIterator.getCurrentValue()
                    looknfeelFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else PyCEGUI.WidgetLookManager.getDefaultResourceGroup())
                    looknfeelRawData = open(looknfeelFilePath, "r").read()
                    looknfeelRawDataType = looknfeel_compatibility.manager.EditorNativeType
                    try:
                        looknfeelRawDataType = looknfeel_compatibility.manager.guessType(looknfeelRawData, looknfeelFilePath)

                    except compatibility.NoPossibleTypesError:
                        QtGui.QMessageBox.warning(None, "LookNFeel doesn't match any known data type", "The looknfeel '%s' wasn't recognised by CEED as any looknfeel data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!" % (looknfeelFilePath))
                        return

                    except compatibility.MultiplePossibleTypesError as e:
                        suitableVersion = looknfeel_compatibility.manager.getSuitableDataTypeForCEGUIVersion(project.CEGUIVersion)

                        if suitableVersion not in e.possibleTypes:
                            QtGui.QMessageBox.warning(None, "Incorrect looknfeel data type", "The looknfeel '%s' checked out as some potential data types, however none of these is suitable for your project's target CEGUI version '%s', please check your project settings! CEGUI instance synchronisation aborted!" % (looknfeelFilePath, suitableVersion))
                            return

                        looknfeelRawDataType = suitableVersion

                    looknfeelNativeData = looknfeel_compatibility.manager.transform(looknfeelRawDataType, looknfeel_compatibility.manager.EditorNativeType, looknfeelRawData)

                    PyCEGUI.WidgetLookManager.getSingleton().parseLookNFeelSpecificationFromString(looknfeelNativeData)
                    looknfeelIterator.next()

                updateProgress("Loading window renderer factory modules")
                scheme.loadWindowRendererFactories()
                updateProgress("Loading window factories")
                scheme.loadWindowFactories()
                updateProgress("Loading factory aliases")
                scheme.loadFactoryAliases()
                updateProgress("Loading falagard mappings")
                scheme.loadFalagardMappings()

        except:
            self.cleanCEGUIResources()
            raise

        finally:
            # put SchemeManager into the default state again
            PyCEGUI.SchemeManager.getSingleton().setAutoLoadResources(True)

            progress.reset()
            QtGui.QApplication.instance().processEvents()
///////////

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

// Destroy all previous resources (if any)
void CEGUIProjectManager::cleanCEGUIResources()
{
    if (!initialized) return;
/*
    PyCEGUI.WindowManager.getSingleton().destroyAllWindows()
    # we need to ensure all windows are destroyed, dangling pointers would
    # make us segfault later otherwise
    PyCEGUI.WindowManager.getSingleton().cleanDeadPool()
    PyCEGUI.FontManager.getSingleton().destroyAll()
    PyCEGUI.ImageManager.getSingleton().destroyAll()
    PyCEGUI.SchemeManager.getSingleton().destroyAll()
    PyCEGUI.WidgetLookManager.getSingleton().eraseAllWidgetLooks()
    PyCEGUI.AnimationManager.getSingleton().destroyAllAnimations()
    PyCEGUI.WindowFactoryManager.getSingleton().removeAllFalagardWindowMappings()
    PyCEGUI.WindowFactoryManager.getSingleton().removeAllWindowTypeAliases()
    PyCEGUI.WindowFactoryManager.getSingleton().removeAllFactories()
    # the previous call removes all Window factories, including the stock ones like DefaultWindow
    # lets add them back
    PyCEGUI.System.getSingleton().addStandardWindowFactories()
    PyCEGUI.System.getSingleton().getRenderer().destroyAllTextures()
*/
}

/*
    def getAvailableSkins(self):
        """Retrieves skins (as strings representing their names) that are available
        from the set of schemes that were loaded.

        see syncToProject
        """

        skins = []

        it = PyCEGUI.WindowFactoryManager.getSingleton().getFalagardMappingIterator()
        while not it.isAtEnd():
            currentSkin = it.getCurrentValue().d_windowType.split('/')[0]

            from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor
            ceedInternalEditingPrefix = LookNFeelTabbedEditor.getEditorIDStringPrefix()
            ceedInternalLNF = False
            if currentSkin.startswith(ceedInternalEditingPrefix):
                ceedInternalLNF = True

            if currentSkin not in skins and not ceedInternalLNF:
                skins.append(currentSkin)

            it.next()

        return sorted(skins)

    def getAvailableFonts(self):
        """Retrieves fonts (as strings representing their names) that are available
        from the set of schemes that were loaded.

        see syncToProject
        """

        fonts = []

        it = PyCEGUI.FontManager.getSingleton().getIterator()
        while not it.isAtEnd():
            fonts.append(it.getCurrentKey())
            it.next()

        return sorted(fonts)

    def getAvailableImages(self):
        """Retrieves images (as strings representing their names) that are available
        from the set of schemes that were loaded.

        see syncToProject
        """

        images = []

        it = PyCEGUI.ImageManager.getSingleton().getIterator()
        while not it.isAtEnd():
            images.append(it.getCurrentKey())
            it.next()

        return sorted(images)

    def getAvailableWidgetsBySkin(self):
        """Retrieves all mappings (string names) of all widgets that can be created

        see syncToProject
        """

        ret = {}
        ret["__no_skin__"] = ["DefaultWindow", "DragContainer",
                              "VerticalLayoutContainer", "HorizontalLayoutContainer",
                              "GridLayoutContainer"]

        it = PyCEGUI.WindowFactoryManager.getSingleton().getFalagardMappingIterator()
        while not it.isAtEnd():
            #base = it.getCurrentValue().d_baseType
            mappedType = it.getCurrentValue().d_windowType.split('/', 1)
            assert(len(mappedType) == 2)

            look = mappedType[0]
            widget = mappedType[1]

            from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor
            ceedInternalEditingPrefix = LookNFeelTabbedEditor.getEditorIDStringPrefix()
            ceedInternalLNF = False
            if look.startswith(ceedInternalEditingPrefix):
                ceedInternalLNF = True

            if not ceedInternalLNF:
                # insert empty list for the look if it's a new look
                if not look in ret:
                    ret[look] = []

                # append widget name to the list for its look
                ret[look].append(widget)

            it.next()

        # sort the lists
        for look in ret:
            ret[look].sort()

        return ret

    def getWidgetPreviewImage(self, widgetType, previewWidth = 128, previewHeight = 64):
        """Renders and retrieves a widget preview image (as QImage).

        This is useful for various widget selection lists as a preview.
        """

        self.ensureCEGUIInitialised()
        self.makeGLContextCurrent()

        system = PyCEGUI.System.getSingleton()

        renderer = system.getRenderer()

        renderTarget = PyCEGUIOpenGLRenderer.OpenGLViewportTarget(renderer)
        renderTarget.setArea(PyCEGUI.Rectf(0, 0, previewWidth, previewHeight))
        renderingSurface = PyCEGUI.RenderingSurface(renderTarget)

        widgetInstance = PyCEGUI.WindowManager.getSingleton().createWindow(widgetType, "preview")
        widgetInstance.setRenderingSurface(renderingSurface)
        # set it's size and position so that it shows up
        widgetInstance.setPosition(PyCEGUI.UVector2(PyCEGUI.UDim(0, 0), PyCEGUI.UDim(0, 0)))
        widgetInstance.setSize(PyCEGUI.USize(PyCEGUI.UDim(0, previewWidth), PyCEGUI.UDim(0, previewHeight)))
        # fake update to ensure everything is set
        widgetInstance.update(1)

        temporaryFBO = QtOpenGL.QGLFramebufferObject(previewWidth, previewHeight, GL.GL_TEXTURE_2D)
        temporaryFBO.bind()

        renderingSurface.invalidate()

        renderer.beginRendering()

        try:
            widgetInstance.render()

        finally:
            # no matter what happens we have to clean after ourselves!
            renderer.endRendering()
            temporaryFBO.release()
            PyCEGUI.WindowManager.getSingleton().destroyWindow(widgetInstance)

        return temporaryFBO.toImage()
*/
