#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/util/DismissableMessage.h"
#include "src/Application.h"
#include "qmessagebox.h"
#include "qprogressdialog.h"
#include "qdiriterator.h"
#include <CEGUI/CEGUI.h>
#ifdef CEED_OPENGL_LEGACY_RENDERER
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#else
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
#endif
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include "qopenglcontext.h"
#include "qoffscreensurface.h"
#include "qopenglframebufferobject.h"
#include "qopenglfunctions.h"

// TODO: one CEGUI widget per editor instead of the global one?
#include "src/ui/CEGUIWidget.h"

QString CEGUIProjectManager::ceguiStringToQString(const CEGUI::String& str)
{
#if (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_8)
	return QString(str.c_str());
#elif (CEGUI_STRING_CLASS == CEGUI_STRING_CLASS_UTF_32)
	return QString(CEGUI::String::convertUtf32ToUtf8(str.c_str()).c_str());
#endif
}

CEGUI::String CEGUIProjectManager::qStringToCeguiString(const QString& str)
{
    return CEGUI::String(str.toLocal8Bit().data());
}

CEGUIProjectManager::CEGUIProjectManager()
{
/*
    self.logger = RedirectingCEGUILogger()
    self.lastRenderTimeDelta = 0
*/

    // TODO: one CEGUI widget per editor instead of the global one?
    ceguiContainerWidget = new CEGUIWidget(qobject_cast<Application*>(qApp)->getMainWindow());
}

CEGUIProjectManager::~CEGUIProjectManager()
{
    if (guiContext) CEGUI::System::getSingleton().destroyGUIContext(*guiContext);
    cleanCEGUIResources();
#ifdef CEED_OPENGL_LEGACY_RENDERER
    CEGUI::OpenGLRenderer::destroySystem();
#else
    CEGUI::OpenGL3Renderer::destroySystem();
#endif
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
                              QString("It seems project at path '%s' doesn't exist or you don't have rights to open it.").arg(filePath));
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

    // Clean resources that were potentially used with this project
    cleanCEGUIResources();

    currentProject->unload();
    currentProject.reset();
}

// Ensures this CEGUI instance is properly initialised, if it's not it initialises it right away
void CEGUIProjectManager::ensureCEGUIInitialized()
{
    if (initialized) return;

    QSurfaceFormat format;
    format.setSamples(0);

    glContext = new QOpenGLContext(); // TODO: destroy
    glContext->setFormat(format);
    if (Q_UNLIKELY(!glContext->create()))
    {
        assert(false);
        return;
    }

    surface = new QOffscreenSurface(glContext->screen());
    surface->setFormat(glContext->format());
    surface->create();

    if (Q_UNLIKELY(!glContext->makeCurrent(surface)))
    {
        //qWarning("QOpenGLWidget: Failed to make context current");
        assert(false);
        return;
    }

    if (!glContext->hasExtension("GL_EXT_framebuffer_object"))
    {
        DismissableMessage::warning(qobject_cast<Application*>(qApp)->getMainWindow(),
                                    "No FBO support!",
                                    "CEED uses OpenGL frame buffer objects for various tasks, "
                                    "most notably to support panning and zooming in the layout editor.\n\n"
                                    "FBO support was not detected on your system!\n\n"
                                    "The editor will run but you may experience rendering artifacts.",
                                    "no_fbo_support");
    }

    // We don't want CEGUI Exceptions to output to stderr every time they are constructed
    CEGUI::Exception::setStdErrEnabled(false);

#ifdef CEED_OPENGL_LEGACY_RENDERER
    CEGUI::OpenGLRenderer* renderer = nullptr;
    try
    {
        //??? glContext->format().version() >= 3.2 -> use OpenGL3Renderer?
        renderer = &CEGUI::OpenGLRenderer::bootstrapSystem(CEGUI::OpenGLRenderer::TextureTargetType::Fbo);
    }
#else
    CEGUI::OpenGL3Renderer* renderer = nullptr;
    try
    {
        renderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    }
#endif
    catch (const std::exception& e)
    {
        QMessageBox::warning(nullptr, "Exception", e.what());
        return;
    }

    // Put the resource groups to a reasonable default value, './datafiles' followed by
    // the respective folder, same as CEGUI stock datafiles
    auto defaultBaseDirectory = QDir(QDir::current().filePath("datafiles"));

    auto resProvider = dynamic_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    if (resProvider)
    {
        resProvider->setResourceGroupDirectory("imagesets", defaultBaseDirectory.filePath("imagesets").toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("fonts", defaultBaseDirectory.filePath("fonts").toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("schemes", defaultBaseDirectory.filePath("schemes").toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("looknfeels", defaultBaseDirectory.filePath("looknfeel").toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("layouts", defaultBaseDirectory.filePath("layouts").toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("xml_schemas", defaultBaseDirectory.filePath("xml_schemas").toLocal8Bit().data());
    }

    // All this will never be set to anything else again
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    //CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
    //CEGUI::AnimationManager::setDefaultResourceGroup("animations");

    auto parser = CEGUI::System::getSingleton().getXMLParser();
    if (parser && parser->isPropertyPresent("SchemaDefaultResourceGroup"))
        parser->setProperty("SchemaDefaultResourceGroup", "xml_schemas");

    // TODO: renderer->get/createViewportTarget!
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer); //CEGUI::Rectf(0.f, 0.f, widthF, heightF)
    guiContext = &CEGUI::System::getSingleton().createGUIContext(*renderTarget);

    initialized = true;
}

// Synchronises the CEGUI instance with the current project, respecting it's paths and resources
bool CEGUIProjectManager::syncProjectToCEGUIInstance()
{
    if (!currentProject)
    {
        cleanCEGUIResources();
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

    QProgressDialog progress(mainWnd);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle("Synchronising embedded CEGUI with the project");
    progress.setCancelButton(nullptr);
    progress.resize(400, 100);
    progress.show();

    ensureCEGUIInitialized();

    QStringList schemeFiles;
    auto absoluteSchemesPath = currentProject->getAbsolutePathOf(currentProject->schemesPath);
    if (!QDir(absoluteSchemesPath).exists())
    {
        progress.reset();
        QMessageBox::warning(mainWnd, "Failed to synchronise embedded CEGUI to your project",
           "Can't list scheme path '" + absoluteSchemesPath + "'\n\n"
           "This means that editing capabilities of CEED will be limited to editing of files "
           "that don't require a project opened (for example: imagesets).");
       return false;
    }

    QDirIterator schemesIt(absoluteSchemesPath);
    while (schemesIt.hasNext())
    {
        schemesIt.next();
        QFileInfo info = schemesIt.fileInfo();
        if (!info.isDir() && info.suffix() == "scheme")
            schemeFiles.append(schemesIt.fileName());
    }

    progress.setMinimum(0);
    progress.setMaximum(2 + 9 * schemeFiles.size());

    progress.setLabelText("Purging all resources...");
    progress.setValue(0);
    QApplication::instance()->processEvents();

    // Destroy all previous resources (if any)
    cleanCEGUIResources();

    progress.setLabelText("Setting resource paths...");
    progress.setValue(1);
    QApplication::instance()->processEvents();

    auto resProvider = dynamic_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    if (resProvider)
    {
        resProvider->setResourceGroupDirectory("imagesets", currentProject->getAbsolutePathOf(currentProject->imagesetsPath).toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("fonts", currentProject->getAbsolutePathOf(currentProject->fontsPath).toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("schemes", currentProject->getAbsolutePathOf(currentProject->schemesPath).toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("looknfeels", currentProject->getAbsolutePathOf(currentProject->looknfeelsPath).toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("layouts", currentProject->getAbsolutePathOf(currentProject->layoutsPath).toLocal8Bit().data());
        resProvider->setResourceGroupDirectory("xml_schemas", currentProject->getAbsolutePathOf(currentProject->xmlSchemasPath).toLocal8Bit().data());
    }

    progress.setLabelText("Recreating all schemes...");
    progress.setValue(2);
    QApplication::instance()->processEvents();

    glContext->makeCurrent(surface);

    // We will load resources manually to be able to use the compatibility layer machinery
    CEGUI::SchemeManager::getSingleton().setAutoLoadResources(false);

    bool result = true;
    try
    {
        auto updateProgress = [&progress](const QString& schemeFile, const QString& message)
        {
            progress.setValue(progress.value() + 1);
            progress.setLabelText(QString("Recreating all schemes... (%1)\n\n%2").arg(schemeFile, message));
            QApplication::instance()->processEvents();
        };

        for (auto& schemeFile : schemeFiles)
        {
            updateProgress(schemeFile, "Parsing the scheme file");

            /*
            auto schemeResourceGroup = CEGUI::String::convertUtf32ToUtf8(CEGUI::Scheme::getDefaultResourceGroup().c_str());
            auto schemeFilePath = currentProject->getResourceFilePath(schemeFile, schemeResourceGroup.c_str());

            rawData = open(schemeFile, "r").read()
            rawDataType = scheme_compatibility.manager.EditorNativeType

            try:
                rawDataType = scheme_compatibility.manager.guessType(rawData, schemeFile)

            except compatibility.NoPossibleTypesError:
                QtGui.QMessageBox.warning(None, "Scheme doesn't match any known data type", "The scheme '%s' wasn't recognised by CEED as any scheme data type known to it. Please check that the data isn't corrupted. CEGUI instance synchronisation aborted!" % (schemeFile))
                return

            except compatibility.MultiplePossibleTypesError as e:
                suitableVersion = scheme_compatibility.manager.getSuitableDataTypeForCEGUIVersion(project.CEGUIVersion)

                if suitableVersion not in e.possibleTypes:
                    QtGui.QMessageBox.warning(None, "Incorrect scheme data type", "The scheme '%s' checked out as some potential data types, however not any of these is suitable for your project's target CEGUI version '%s', please check your project settings! CEGUI instance synchronisation aborted!" % (schemeFile, suitableVersion))
                    return

                rawDataType = suitableVersion

            nativeData = scheme_compatibility.manager.transform(rawDataType, scheme_compatibility.manager.EditorNativeType, rawData)
            scheme = CEGUI::SchemeManager::getSingleton().createFromString(nativeData)
            */

            CEGUI::Scheme& scheme = CEGUI::SchemeManager::getSingleton().createFromFile(schemeFile.toLocal8Bit().data());

            // NOTE: This is very CEGUI implementation specific unfortunately!
            //       However I am not really sure how to do this any better.
            updateProgress(schemeFile, "Loading XML imagesets");

            auto xmlImagesetIterator = scheme.getXMLImagesets();
            while (!xmlImagesetIterator.isAtEnd())
            {
                auto loadableUIElement = xmlImagesetIterator.getCurrentValue();

                /*
                imagesetFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else CEGUI::ImageManager.getImagesetDefaultResourceGroup())
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

                CEGUI::ImageManager::getSingleton().loadImagesetFromString(imagesetNativeData)
                */

                CEGUI::ImageManager::getSingleton().loadImageset(loadableUIElement.filename, loadableUIElement.resourceGroup);

                ++xmlImagesetIterator;
            }

            updateProgress(schemeFile, "Loading image file imagesets");

            scheme.loadImageFileImagesets();

            updateProgress(schemeFile, "Loading fonts");

            auto fontIterator = scheme.getFonts();
            while (!fontIterator.isAtEnd())
            {
                auto loadableUIElement = fontIterator.getCurrentValue();

                /*
                fontFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else CEGUI::Font.getDefaultResourceGroup())
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

                CEGUI::FontManager::getSingleton().createFromString(fontNativeData)
                */

                CEGUI::FontManager::getSingleton().createFromFile(loadableUIElement.filename, loadableUIElement.resourceGroup);

                ++fontIterator;
            }

            updateProgress(schemeFile, "Loading looknfeels");

            auto looknfeelIterator = scheme.getLookNFeels();
            while (!looknfeelIterator.isAtEnd())
            {
                auto loadableUIElement = looknfeelIterator.getCurrentValue();

                /*
                looknfeelFilePath = project.getResourceFilePath(loadableUIElement.filename, loadableUIElement.resourceGroup if loadableUIElement.resourceGroup != "" else CEGUI::WidgetLookManager.getDefaultResourceGroup())
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

                CEGUI::WidgetLookManager::getSingleton().parseLookNFeelSpecificationFromString(looknfeelNativeData)
                */

                CEGUI::WidgetLookManager::getSingleton().parseLookNFeelSpecificationFromFile(loadableUIElement.filename, loadableUIElement.resourceGroup);

                ++looknfeelIterator;
            }

            updateProgress(schemeFile, "Loading window renderer factory modules");
            scheme.loadWindowRendererFactories();
            updateProgress(schemeFile, "Loading window factories");
            scheme.loadWindowFactories();
            updateProgress(schemeFile, "Loading factory aliases");
            scheme.loadFactoryAliases();
            updateProgress(schemeFile, "Loading falagard mappings");
            scheme.loadFalagardMappings();
        }
    }
    catch (const std::exception& e)
    {
        cleanCEGUIResources();
        QMessageBox::warning(mainWnd, "Failed to synchronise embedded CEGUI to your project",
            QString("An attempt was made to load resources related to the project being opened, "
            "for some reason the loading didn't succeed so all resources were destroyed! "
            "The most likely reason is that the resource directories are wrong, this can "
            "be very easily remedied in the project settings.\n\n"
            "This means that editing capabilities of CEED will be limited to editing of files "
            "that don't require a project opened (for example: imagesets).\nException: %1").arg(e.what()));
        result = false;
    }

    // Put SchemeManager into the default state again
    CEGUI::SchemeManager::getSingleton().setAutoLoadResources(true);

    glContext->doneCurrent();

    progress.reset();
    QApplication::instance()->processEvents();

    return result;
}

// Destroy all previous resources (if any)
void CEGUIProjectManager::cleanCEGUIResources()
{
    if (!initialized) return;

    glContext->makeCurrent(surface);

    CEGUI::WindowManager::getSingleton().destroyAllWindows();

    // We need to ensure all windows are destroyed, dangling pointers would make us segfault later otherwise
    CEGUI::WindowManager::getSingleton().cleanDeadPool();
    CEGUI::FontManager::getSingleton().destroyAll();
    CEGUI::ImageManager::getSingleton().destroyAll();
    CEGUI::SchemeManager::getSingleton().destroyAll();
    CEGUI::WidgetLookManager::getSingleton().eraseAllWidgetLooks();
    CEGUI::AnimationManager::getSingleton().destroyAllAnimations();
    CEGUI::WindowFactoryManager::getSingleton().removeAllFalagardWindowMappings();
    CEGUI::WindowFactoryManager::getSingleton().removeAllWindowTypeAliases();
    CEGUI::WindowFactoryManager::getSingleton().removeAllFactories();

    // The previous call removes all Window factories, including the stock ones like DefaultWindow. Lets add them back.
    CEGUI::System::getSingleton().addStandardWindowFactories();
    CEGUI::System::getSingleton().getRenderer()->destroyAllTextures();

    glContext->doneCurrent();
}

// Retrieves names of skins that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIProjectManager::getAvailableSkins() const
{
    QStringList skins;

    auto it = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
    while (!it.isAtEnd())
    {
        /*
            currentSkin = it.getCurrentValue().d_windowType.split('/')[0]

            from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor
            ceedInternalEditingPrefix = LookNFeelTabbedEditor.getEditorIDStringPrefix()
            ceedInternalLNF = False
            if currentSkin.startswith(ceedInternalEditingPrefix):
                ceedInternalLNF = True

            if currentSkin not in skins and not ceedInternalLNF:
                skins.append(currentSkin)
        */
        ++it;
    }

    std::sort(skins.begin(), skins.end());
    return skins;
}

// Retrieves names of fonts that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIProjectManager::getAvailableFonts() const
{
    QStringList fonts;

    auto& fontRegistry = CEGUI::FontManager::getSingleton().getRegisteredFonts();
    for (const auto& pair : fontRegistry)
    {
        fonts.append(ceguiStringToQString(pair.first));
    }

    std::sort(fonts.begin(), fonts.end());
    return fonts;
}

// Retrieves names of images that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIProjectManager::getAvailableImages() const
{
    QStringList images;

    auto it = CEGUI::ImageManager::getSingleton().getIterator();
    while (!it.isAtEnd())
    {
        images.append(ceguiStringToQString(it.getCurrentKey()));
        ++it;
    }

    std::sort(images.begin(), images.end());
    return images;
}

// Retrieves all mappings (string names) of all widgets that can be created
// see syncProjectToCEGUIInstance
void CEGUIProjectManager::getAvailableWidgetsBySkin(std::map<QString, QStringList>& out) const
{
    out["__no_skin__"].append(
    {
        "DefaultWindow",
        "DragContainer",
        "VerticalLayoutContainer",
        "HorizontalLayoutContainer",
        "GridLayoutContainer"
    });

    const QString ceedInternalEditingPrefix = getEditorIDStringPrefix();

    auto it = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
    while (!it.isAtEnd())
    {
        const QString windowType = ceguiStringToQString(it.getCurrentValue().d_windowType);

        auto sepPos = windowType.indexOf('/');
        assert(sepPos >= 0);
        const QString look = windowType.left(sepPos);
        const QString widget = windowType.mid(sepPos + 1);
        if (!look.startsWith(ceedInternalEditingPrefix))
            out[look].append(widget);

        ++it;
    }

    for (auto& pair : out)
        pair.second.sort();
}

// Renders and retrieves a widget preview QImage. This is useful for various widget selection lists as a preview.
QImage CEGUIProjectManager::getWidgetPreviewImage(const QString& widgetType, int previewWidth, int previewHeight)
{
    ensureCEGUIInitialized();

    const float previewWidthF = static_cast<float>(previewWidth);
    const float previewHeightF = static_cast<float>(previewHeight);

    //???allocate previews once?
    // TODO: renderer->get/createViewportTarget!
#ifdef CEED_OPENGL_LEGACY_RENDERER
    auto renderer = static_cast<CEGUI::OpenGLRenderer*>(CEGUI::System::getSingleton().getRenderer());
#else
    auto renderer = static_cast<CEGUI::OpenGL3Renderer*>(CEGUI::System::getSingleton().getRenderer());
#endif
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, CEGUI::Rectf(0.f, 0.f, previewWidthF, previewHeightF));

    auto renderingSurface = new CEGUI::RenderingSurface(*renderTarget);

    auto widgetInstance = CEGUI::WindowManager::getSingleton().createWindow(widgetType.toLocal8Bit().data(), "preview");

    widgetInstance->setRenderingSurface(renderingSurface);

    // Set it's size and position so that it shows up
    // TODO: per-widget-type size! See WidgetsSample!
    widgetInstance->setPosition(CEGUI::UVector2(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)));
    widgetInstance->setSize(CEGUI::USize(CEGUI::UDim(0.f, previewWidthF), CEGUI::UDim(0.f, previewHeightF)));

    // Window is not attached to a context so it has no default font. Set default.
    // TODO: if project has no fonts, create CEED-internal default font.
    if (!widgetInstance->getFont())
    {
        const auto& fontRegistry = CEGUI::FontManager::getSingleton().getRegisteredFonts();
        CEGUI::Font* defaultFont = fontRegistry.empty() ? nullptr : fontRegistry.begin()->second;
        widgetInstance->setFont(defaultFont);
    }

    CEGUI::Spinner* spinner = dynamic_cast<CEGUI::Spinner*>(widgetInstance);
    widgetInstance->setText(spinner ? "0" : widgetType.toLocal8Bit().data());

    // Fake update to ensure everything is set
    widgetInstance->update(1.f);

    glContext->makeCurrent(surface);

    //???allocate once?
    auto temporaryFBO = new QOpenGLFramebufferObject(previewWidth, previewHeight);
    temporaryFBO->bind();

    glContext->functions()->glClearColor(0.9f, 0.9f, 0.9f, 1.f);
    glContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    renderer->beginRendering();

    QString error;
    try
    {
        widgetInstance->draw();
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    renderer->endRendering();
    temporaryFBO->release();

    QImage result = temporaryFBO->toImage();

    delete temporaryFBO;
    CEGUI::WindowManager::getSingleton().destroyWindow(widgetInstance);
    delete renderingSurface;
    delete renderTarget;

    glContext->doneCurrent();

    if (!error.isEmpty())
        throw error;

    return result;
}
