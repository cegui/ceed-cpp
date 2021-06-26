#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/cegui/QtnPropertyUDim.h"
#include "src/cegui/QtnPropertyUVector2.h"
#include "src/cegui/QtnPropertyUVector3.h"
#include "src/cegui/QtnPropertyUSize.h"
#include "src/cegui/QtnPropertyURect.h"
#include "src/cegui/QtnPropertyUBox.h"
#include "src/ui/CEGUIDebugInfo.h"
#include "src/util/DismissableMessage.h"
#include "src/util/Utils.h"
#include "src/Application.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/FreeTypeFont.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include "QtnProperty/Enum.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include "qmessagebox.h"
#include "qprogressdialog.h"
#include "qdiriterator.h"
#include "qopenglcontext.h"
#include "qoffscreensurface.h"
#include "qopenglframebufferobject.h"
#include "qopenglfunctions.h"
#include <qopenglfunctions_3_2_core.h>
#include <qdom.h>
#include <qtextstream.h>

// Allows us to register subscribers that want CEGUI log info
// This prevents writing CEGUI.log into CWD and allow log display inside the app
class RedirectingCEGUILogger : public CEGUI::Logger
{
public:

    void subscribe(std::function<void(const CEGUI::String&, CEGUI::LoggingLevel)> callback)
    {
        if (callback) callbacks.push_back(callback);
    }

    void unsubscribe(std::function<void(const CEGUI::String&, CEGUI::LoggingLevel)> /*callback*/)
    {
        // TODO: implement
        assert(false);
    }

    void unsubscribeAll() { callbacks.clear(); }

    virtual void logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level = CEGUI::LoggingLevel::Standard) override
    {
        for (auto&& callback : callbacks)
            callback(message, level);
    }

    virtual void setLogFilename(const CEGUI::String&, bool) override {}

private:

    std::vector<std::function<void(const CEGUI::String&, CEGUI::LoggingLevel)>> callbacks;
};

CEGUIManager::CEGUIManager()
{
    _listItemModel.addItem("item 1");
    _listItemModel.addItem("item 2");
    _listItemModel.addItem("item 3");
    _listItemModel.addItem("item 4");
}

CEGUIManager::~CEGUIManager()
{
    if (initialized)
    {
        logger->unsubscribeAll();
        cleanCEGUIResources();
        if (_isOpenGL3)
            CEGUI::OpenGL3Renderer::destroySystem();
        else
            CEGUI::OpenGLRenderer::destroySystem();
        delete debugInfo;
        delete logger;
        delete glContext;
    }

    delete _enumHorizontalAlignment;
    delete _enumVerticalAlignment;
    delete _enumAspectMode;
    delete _enumDefaultParagraphDirection;
    delete _enumWindowUpdateMode;
    delete _enumHorizontalFormatting;
    delete _enumVerticalFormatting;
    delete _enumHorizontalTextFormatting;
    delete _enumVerticalTextFormatting;
}

CEGUIProject* CEGUIManager::createProject(const QString& filePath, bool createResourceDirs)
{
    if (isProjectLoaded())
    {
        QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                              "Error when creating project",
                              "There is another project opened. Close it before creating a new one.");
        return nullptr;
    }

    currentProject.reset(new CEGUIProject());
    currentProject->filePath = filePath;

    // Enforce the "project" extension
    auto ext = "." + ceedProjectExtension();
    if (!currentProject->filePath.endsWith(ext))
        currentProject->filePath += ext;

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

    // Save to disk immediately
    currentProject->save();

    // NB: syncProjectToCEGUIInstance() is not called because it will be called
    // after the initial project setup in a project settings dialog.

    return currentProject.get();
}

// Opens the project file given in 'path'. Assumes no project is opened at the point this is called.
// Caller must test if a project is opened and close it accordingly (with a dialog
// being shown if there are changes to it)
// Errors aren't indicated by exceptions or return values, dialogs are shown in case of errors.
void CEGUIManager::loadProject(const QString& filePath)
{
    if (isProjectLoaded())
    {
        QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                              "Error when opening project",
                              "There is another project opened. Close it before opening another one.");
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

    syncProjectToCEGUIInstance();
}

// Closes currently opened project. Assumes the one is opened at the point this is called.
void CEGUIManager::unloadProject()
{
    if (!currentProject) return;

    // Clean resources that were potentially used with this project
    cleanCEGUIResources();

    _widgetPreviewCache.clear();

    currentProject->unload();
    currentProject.reset();
}

// Ensures this CEGUI instance is properly initialised, if it's not it initialises it right away
void CEGUIManager::ensureCEGUIInitialized()
{
    if (initialized) return;

    glContext = new QOpenGLContext();
    glContext->setShareContext(QOpenGLContext::globalShareContext());
    if (Q_UNLIKELY(!glContext->create()))
    {
        assert(false);
        return;
    }

    _isOpenGL3 = (glContext->versionFunctions<QOpenGLFunctions_3_2_Core>() != nullptr);

    surface = new QOffscreenSurface(glContext->screen());
    surface->setFormat(glContext->format());
    surface->create();

    if (Q_UNLIKELY(!makeOpenGLContextCurrent()))
    {
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

    logger = new RedirectingCEGUILogger();
    debugInfo = new CEGUIDebugInfo();

    logger->subscribe([this](const CEGUI::String& message, CEGUI::LoggingLevel level)
    {
        debugInfo->logEvent(message, level);
    });

    CEGUI::OpenGLRendererBase* renderer = nullptr;
    try
    {
        if (_isOpenGL3)
            renderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
        else
            renderer = &CEGUI::OpenGLRenderer::bootstrapSystem();
    }
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
        resProvider->setResourceGroupDirectory("imagesets", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("imagesets")));
        resProvider->setResourceGroupDirectory("fonts", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("fonts")));
        resProvider->setResourceGroupDirectory("schemes", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("schemes")));
        resProvider->setResourceGroupDirectory("looknfeels", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("looknfeel")));
        resProvider->setResourceGroupDirectory("layouts", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("layouts")));
        resProvider->setResourceGroupDirectory("xml_schemas", CEGUIUtils::qStringToString(defaultBaseDirectory.filePath("xml_schemas")));
        resProvider->setResourceGroupDirectory("__ceed_internal__", CEGUIUtils::qStringToString(QDir::current().path()));
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

    // Must be done once!
    QtnPropertyDelegateUDim::Register(QtnPropertyDelegateFactory::staticInstance());
    QtnPropertyDelegateUVector2::Register(QtnPropertyDelegateFactory::staticInstance());
    QtnPropertyDelegateUVector3::Register(QtnPropertyDelegateFactory::staticInstance());
    QtnPropertyDelegateUSize::Register(QtnPropertyDelegateFactory::staticInstance());
    QtnPropertyDelegateURect::Register(QtnPropertyDelegateFactory::staticInstance());
    QtnPropertyDelegateUBox::Register(QtnPropertyDelegateFactory::staticInstance());

    initialized = true;
}

bool CEGUIManager::makeOpenGLContextCurrent()
{
    return glContext ? glContext->makeCurrent(surface) : false;
}

void CEGUIManager::doneOpenGLContextCurrent()
{
    if (glContext) glContext->doneCurrent();
}

void CEGUIManager::showDebugInfo()
{
    if (debugInfo)
        debugInfo->show();
    else
        QMessageBox::warning(nullptr, "CEGUI Debug Info", "CEGUI is not initialized yet. Open a project to launch it.");
}

// Synchronises the CEGUI instance with the current project, respecting it's paths and resources
bool CEGUIManager::syncProjectToCEGUIInstance()
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
        resProvider->setResourceGroupDirectory("imagesets", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->imagesetsPath)));
        resProvider->setResourceGroupDirectory("fonts", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->fontsPath)));
        resProvider->setResourceGroupDirectory("schemes", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->schemesPath)));
        resProvider->setResourceGroupDirectory("looknfeels", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->looknfeelsPath)));
        resProvider->setResourceGroupDirectory("layouts", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->layoutsPath)));
        resProvider->setResourceGroupDirectory("xml_schemas", CEGUIUtils::qStringToString(currentProject->getAbsolutePathOf(currentProject->xmlSchemasPath)));
        resProvider->setResourceGroupDirectory("__ceed_internal__", CEGUIUtils::qStringToString(QDir::current().path()));
    }

    progress.setLabelText("Recreating all schemes...");
    progress.setValue(2);
    QApplication::instance()->processEvents();

    makeOpenGLContextCurrent();

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
            auto schemeResourceGroup = CEGUIUtils::stringToQString(CEGUI::Scheme::getDefaultResourceGroup());
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

            CEGUI::Scheme& scheme = CEGUI::SchemeManager::getSingleton().createFromFile(CEGUIUtils::qStringToString(schemeFile));

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

    doneOpenGLContextCurrent();

    progress.reset();
    QApplication::instance()->processEvents();

    return result;
}

// Destroy all previous resources (if any)
void CEGUIManager::cleanCEGUIResources()
{
    if (!initialized) return;

    makeOpenGLContextCurrent();

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

    doneOpenGLContextCurrent();
}

// Retrieves names of skins that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIManager::getAvailableSkins() const
{
    QStringList skins;

    auto it = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
    while (!it.isAtEnd())
    {
        QString currentSkin = CEGUIUtils::stringToQString(it.getCurrentValue().d_windowType);
        auto sepPos = currentSkin.indexOf('/');
        if (sepPos >= 0) currentSkin = currentSkin.left(sepPos);

        if (!skins.contains(currentSkin) && !currentSkin.startsWith(getEditorIDStringPrefix()))
            skins.append(currentSkin);

        ++it;
    }

    std::sort(skins.begin(), skins.end());
    return skins;
}

// Retrieves names of fonts that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIManager::getAvailableFonts() const
{
    QStringList fonts;

    auto& fontRegistry = CEGUI::FontManager::getSingleton().getRegisteredFonts();
    for (const auto& pair : fontRegistry)
        if (pair.first != "__CEEDDefaultFont__")
            fonts.append(CEGUIUtils::stringToQString(pair.first));

    std::sort(fonts.begin(), fonts.end());
    return fonts;
}

bool CEGUIManager::saveFont(CEGUI::Font& font, bool addToSchemes) const
{
    const QString fontDescFileName = CEGUIUtils::stringToQString(font.getName()) + ".font";

    // Save an XML font description to the project
    {
        const auto dstFontDescPath = currentProject->getResourceFilePath(fontDescFileName, "fonts");
        std::ofstream stream(dstFontDescPath.toStdString(), std::ios_base::out | std::ios_base::trunc);
        CEGUI::FontManager::getSingleton().writeFontToStream(font.getName(), stream);
    }

    // Add a new font to all schemes

    if (!addToSchemes) return true;

    const auto absoluteSchemesPath = currentProject->getAbsolutePathOf(currentProject->schemesPath);
    if (!QDir(absoluteSchemesPath).exists()) return true;

    QDirIterator schemesIt(absoluteSchemesPath);
    while (schemesIt.hasNext())
    {
        schemesIt.next();
        QFileInfo info = schemesIt.fileInfo();
        if (info.isDir() || info.suffix() != "scheme") continue;

        QDomDocument doc;

        // Open, read & close file. We will work with a DOM document.
        {
            QFile file(info.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return false;

            if (!doc.setContent(&file))
                return false;
        }

        auto xmlGUIScheme = doc.documentElement();
        QDomElement xmlFontRec = doc.createElement("Font");
        xmlFontRec.setAttribute("filename", fontDescFileName);
        xmlGUIScheme.appendChild(xmlFontRec);

        // Write XML back with changes
        {
            QFile file(info.absoluteFilePath());
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

            QTextStream stream(&file);
            doc.save(stream, 4);
            file.close();
        }
    }

    return true;
}

// Retrieves names of images that are available from the set of schemes that were loaded.
// see syncProjectToCEGUIInstance
QStringList CEGUIManager::getAvailableImages() const
{
    QStringList images;

    auto it = CEGUI::ImageManager::getSingleton().getIterator();
    while (!it.isAtEnd())
    {
        if (it.getCurrentKey() != "__CEEDDefaultImage__")
            images.append(CEGUIUtils::stringToQString(it.getCurrentKey()));
        ++it;
    }

    std::sort(images.begin(), images.end());
    return images;
}

// Retrieves all mappings (string names) of all widgets that can be created
// see syncProjectToCEGUIInstance
void CEGUIManager::getAvailableWidgetsBySkin(std::map<QString, QStringList>& out) const
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
        const QString windowType = CEGUIUtils::stringToQString(it.getCurrentValue().d_windowType);

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

// See Widgets example from CEGUI
void CEGUIManager::initializePreviewWidgetSpecific(CEGUI::Window* widgetInstance, const QString& widgetType)
{
    if (auto spinner = dynamic_cast<CEGUI::Spinner*>(widgetInstance))
        widgetInstance->setText("123");
    else if (dynamic_cast<CEGUI::MultiLineEditbox*>(widgetInstance) || widgetType.endsWith("StaticText"))
        widgetInstance->setText(CEGUIUtils::qStringToString(widgetType + "\nMultiline multiline multiline multiline"));
    else
        widgetInstance->setText(CEGUIUtils::qStringToString(widgetType));

    if (widgetType.endsWith("StaticText"))
    {
        if (widgetInstance->isPropertyPresent("VertScrollbar"))
            widgetInstance->setProperty("VertScrollbar", "true");

        if (widgetInstance->isPropertyPresent("HorzFormatting"))
            widgetInstance->setProperty("HorzFormatting", "WordWrapLeftAligned");

        if (widgetType.endsWith("CaptionedStaticText"))
            widgetInstance->setProperty("Text", "Caption");
    }
    else if (widgetType.endsWith("StaticImage"))
    {
        if (!CEGUI::ImageManager::getSingleton().isDefined("__CEEDDefaultImage__"))
            CEGUI::ImageManager::getSingleton().addBitmapImageFromFile("__CEEDDefaultImage__", "data/images/ceed.png", "__ceed_internal__");
        widgetInstance->setProperty("Image", "__CEEDDefaultImage__");
    }
    else if (widgetType.endsWith("ImageButton"))
    {
        if (!CEGUI::ImageManager::getSingleton().isDefined("__CEEDDefaultImage__"))
            CEGUI::ImageManager::getSingleton().addBitmapImageFromFile("__CEEDDefaultImage__", "data/images/ceed.png", "__ceed_internal__");
        widgetInstance->setProperty("NormalImage", "__CEEDDefaultImage__");
    }
    else if (auto comboBox = dynamic_cast<CEGUI::Combobox*>(widgetInstance))
    {
        comboBox->getDropList()->show();
        comboBox->getDropList()->setModel(&_listItemModel);
    }
    else if (auto listView = dynamic_cast<CEGUI::ListView*>(widgetInstance))
    {
        listView->setModel(&_listItemModel);
        if (listView->getType().find("WindowsLook/List") == 0)
            listView->setTextColour(CEGUI::Colour(0.0f, 0.0f, 0.0f, 1.0f));
    }
    else if (auto listWidget = dynamic_cast<CEGUI::ListWidget*>(widgetInstance))
    {
        listWidget->addItem("ListWidget Item 1");
        listWidget->addItem("ListWidget Item 2");
        listWidget->addItem("ListWidget Item 3");
        listWidget->addItem("ListWidget Item 4");
        if (listWidget->getType().compare("WindowsLook/ListWidget") == 0)
            listWidget->setTextColour(CEGUI::Colour(0.0f, 0.0f, 0.0f, 1.0f));
    }
    else if (auto multiColumnList = dynamic_cast<CEGUI::MultiColumnList*>(widgetInstance))
    {
        widgetInstance->setSize(CEGUI::USize(CEGUI::UDim(0.f, 400.f), CEGUI::UDim(0.f, 160.f)));

        multiColumnList->addColumn("Server Name", 0, cegui_reldim(0.38f));
        multiColumnList->addColumn("Address ", 1, cegui_reldim(0.44f));
        multiColumnList->addColumn("Ping", 2, cegui_reldim(0.15f));

        multiColumnList->addRow();
        multiColumnList->addRow();

        multiColumnList->setItem(new CEGUI::ListboxTextItem("Laggers World"), 0, 0);
        multiColumnList->setItem(new CEGUI::ListboxTextItem("yourgame.some-server.com"), 1, 0);
        auto item = new CEGUI::ListboxTextItem("[colour='FFFF0000']1000ms");
        item->setTextParsingEnabled(true);
        multiColumnList->setItem(item, 2, 0);

        multiColumnList->setItem(new CEGUI::ListboxTextItem("Super-Server"), 0, 1);
        multiColumnList->setItem(new CEGUI::ListboxTextItem("whizzy.fakenames.net"), 1, 1);
        item = new CEGUI::ListboxTextItem("[colour='FF00FF00']8ms");
        item->setTextParsingEnabled(true);
        multiColumnList->setItem(item, 2, 1);
    }
    else if (auto menuBar = dynamic_cast<CEGUI::Menubar*>(widgetInstance))
    {
        CEGUI::String skin = menuBar->getType();
        skin = skin.substr(0, skin.find_first_of('/'));
        CEGUI::String menuItemMapping = skin + "/MenuItem";

        CEGUI::WindowManager& windowManager = CEGUI::WindowManager::getSingleton();
        CEGUI::MenuItem* fileMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "__CEED_FileMenuItem__"));
        fileMenuItem->setFont(menuBar->getFont());
        fileMenuItem->setText("File");
        menuBar->addChild(fileMenuItem);

        CEGUI::MenuItem* viewMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "__CEED_ViewMenuItem__"));
        viewMenuItem->setFont(menuBar->getFont());
        viewMenuItem->setText("View");
        menuBar->addChild(viewMenuItem);
    }
    else if (auto progressBar = dynamic_cast<CEGUI::ProgressBar*>(widgetInstance))
    {
        progressBar->setProgress(0.5f);
    }
}

// Renders and retrieves a widget preview QImage. This is useful for various widget selection lists as a preview.
const QImage& CEGUIManager::getWidgetPreviewImage(const QString& widgetType, int previewWidth, int previewHeight)
{
    auto it = _widgetPreviewCache.find(widgetType);
    if (it != _widgetPreviewCache.cend()) return it->second;

    ensureCEGUIInitialized();

    auto widgetInstance = CEGUI::WindowManager::getSingleton().createWindow(CEGUIUtils::qStringToString(widgetType), "preview");
    widgetInstance->setVisible(true);

    // Setup default size
    {
        const auto size = widgetInstance->calculatePixelSize();
        float previewWidthF = previewWidth ? static_cast<float>(previewWidth) : size.d_width;
        float previewHeightF = previewHeight ? static_cast<float>(previewHeight) : size.d_height;

        if (previewWidthF < 1.f) previewWidthF = 256.f;
        if (previewHeightF < 1.f) previewHeightF = 128.f;

        if (previewWidthF < 100.f || previewHeightF < 50.f)
        {
            previewWidthF *= 1.5f;
            previewHeightF *= 1.5f;
        }

        widgetInstance->setPosition(CEGUI::UVector2(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)));
        widgetInstance->setSize(CEGUI::USize(CEGUI::UDim(0.f, previewWidthF), CEGUI::UDim(0.f, previewHeightF)));
    }

    // Window is not attached to a context so it has no default font. Set default.
    if (!widgetInstance->getFont())
    {
        auto& fontManager = CEGUI::FontManager::getSingleton();
        const auto& fontRegistry = fontManager.getRegisteredFonts();

        // If project has no fonts, create CEED-internal default font
        if (fontRegistry.empty())
        {
            const QSizeF resolution = currentProject ? currentProject->getDefaultResolution() : QSizeF(1280.0, 720.0);
            fontManager.createFreeTypeFont("__CEEDDefaultFont__", 14.f, CEGUI::FontSizeUnit::Pixels,
                true, "data/fonts/DejaVuSans.ttf", "__ceed_internal__",
                CEGUI::AutoScaledMode::Disabled,
                CEGUI::Sizef(static_cast<float>(resolution.width()), static_cast<float>(resolution.height())));
        }

        CEGUI::Font* defaultFont = fontRegistry.empty() ? nullptr : fontRegistry.begin()->second;
        widgetInstance->setFont(defaultFont);

        // FIXME: FrameWindow has a Titlebar which uses own font. Teach it to get default from the parent FrameWindow?
        if (auto frameWnd = dynamic_cast<CEGUI::FrameWindow*>(widgetInstance))
            if (frameWnd->getTitlebar())
                frameWnd->getTitlebar()->setFont(defaultFont);
    }

    // Some widgets look better after some initial setup
    initializePreviewWidgetSpecific(widgetInstance, widgetType);

    // Fake update to ensure everything is set
    widgetInstance->update(1.f);

    // Size could change, use an actual one
    const auto size = widgetInstance->calculatePixelSize();

    //???allocate previews once?
    // TODO: renderer->get/createViewportTarget!
    auto renderer = static_cast<CEGUI::OpenGLRendererBase*>(CEGUI::System::getSingleton().getRenderer());
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, CEGUI::Rectf(0.f, 0.f, size.d_width, size.d_height));
    auto renderingSurface = new CEGUI::RenderingSurface(*renderTarget);
    widgetInstance->setRenderingSurface(renderingSurface);

    makeOpenGLContextCurrent();

    // Create FBO of an actual object size
    auto temporaryFBO = new QOpenGLFramebufferObject(static_cast<int>(size.d_width), static_cast<int>(size.d_height));
    temporaryFBO->bind();

    glContext->functions()->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

    doneOpenGLContextCurrent();

    if (!error.isEmpty())
        throw error;

    Utils::fillTransparencyWithChecker(result);

    return _widgetPreviewCache.emplace(widgetType, std::move(result)).first->second;
}

const QtnEnumInfo& CEGUIManager::enumHorizontalAlignment()
{
    // TODO: request to Qtn - more convenient static enum declaration / example
    /*
    QtnEnumInfo info("HorizontalAlignment",
    {
        QtnEnumValueInfo(static_cast<QtnEnumValueType>(CEGUI::HorizontalAlignment::Left), "Left")
    });
    */

    if (!_enumHorizontalAlignment)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalAlignment::Left), "Left"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalAlignment::Centre), "Center"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalAlignment::Right), "Right"});
        _enumHorizontalAlignment = new QtnEnumInfo("HorizontalAlignment", values);
    }
    return *_enumHorizontalAlignment;
}

const QtnEnumInfo& CEGUIManager::enumVerticalAlignment()
{
    if (!_enumVerticalAlignment)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalAlignment::Top), "Top"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalAlignment::Centre), "Center"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalAlignment::Bottom), "Bottom"});
        _enumVerticalAlignment = new QtnEnumInfo("VerticalAlignment", values);
    }
    return *_enumVerticalAlignment;
}

const QtnEnumInfo& CEGUIManager::enumAspectMode()
{
    if (!_enumAspectMode)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::AspectMode::Expand), "Expand"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::AspectMode::Ignore), "Ignore"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::AspectMode::Shrink), "Shrink"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::AspectMode::AdjustWidth), "AdjustWidth", "Adjust width"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::AspectMode::AdjustHeight), "AdjustHeight", "Adjust height"});
        _enumAspectMode = new QtnEnumInfo("AspectMode", values);
    }
    return *_enumAspectMode;
}

const QtnEnumInfo& CEGUIManager::enumDefaultParagraphDirection()
{
    if (!_enumDefaultParagraphDirection)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::DefaultParagraphDirection::Automatic), "Automatic"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::DefaultParagraphDirection::LeftToRight), "LeftToRight", "Left to right"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::DefaultParagraphDirection::RightToLeft), "RightToLeft", "Right to left"});
        _enumDefaultParagraphDirection = new QtnEnumInfo("DefaultParagraphDirection", values);
    }
    return *_enumDefaultParagraphDirection;
}

const QtnEnumInfo& CEGUIManager::enumWindowUpdateMode()
{
    if (!_enumWindowUpdateMode)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::WindowUpdateMode::Never), "Never"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::WindowUpdateMode::Always), "Always"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::WindowUpdateMode::Visible), "Visible"});
        _enumWindowUpdateMode = new QtnEnumInfo("WindowUpdateMode", values);
    }
    return *_enumWindowUpdateMode;
}

const QtnEnumInfo& CEGUIManager::enumHorizontalFormatting()
{
    if (!_enumHorizontalFormatting)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalFormatting::Tiled), "Tiled", "Tile"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalFormatting::Stretched), "Stretched", "Stretch"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalFormatting::LeftAligned), "LeftAligned", "Left"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalFormatting::RightAligned), "RightAligned", "Right"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalFormatting::CentreAligned), "CentreAligned", "Center"});
        _enumHorizontalFormatting = new QtnEnumInfo("HorizontalFormatting", values);
    }
    return *_enumHorizontalFormatting;
}

const QtnEnumInfo& CEGUIManager::enumVerticalFormatting()
{
    if (!_enumVerticalFormatting)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalImageFormatting::Tiled), "Tiled", "Tile"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalImageFormatting::Stretched), "Stretched", "Stretch"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalImageFormatting::TopAligned), "TopAligned"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalImageFormatting::BottomAligned), "BottomAligned", "Bottom"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalImageFormatting::CentreAligned), "CentreAligned", "Center"});
        _enumVerticalFormatting = new QtnEnumInfo("VerticalFormatting", values);
    }
    return *_enumVerticalFormatting;
}

//???TODO: make special property of alignment + word wrap flag?
const QtnEnumInfo& CEGUIManager::enumHorizontalTextFormatting()
{
    if (!_enumHorizontalTextFormatting)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::Justified), "Justified"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::LeftAligned), "LeftAligned", "Left"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::RightAligned), "RightAligned", "Right"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::CentreAligned), "CentreAligned", "Center"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::WordWraperJustified), "WordWraperJustified", "Justified"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::WordWrapLeftAligned), "WordWrapLeftAligned", "Left word-wrapped"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::WordWrapRightAligned), "WordWrapRightAligned", "Right word-wrapped"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::HorizontalTextFormatting::WordWrapCentreAligned), "WordWrapCentreAligned", "Center word-wrapped"});
        _enumHorizontalTextFormatting = new QtnEnumInfo("HorizontalTextFormatting", values);
    }
    return *_enumHorizontalTextFormatting;
}

const QtnEnumInfo& CEGUIManager::enumVerticalTextFormatting()
{
    if (!_enumVerticalTextFormatting)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalTextFormatting::TopAligned), "TopAligned", "Top"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalTextFormatting::BottomAligned), "BottomAligned", "Bottom"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::VerticalTextFormatting::CentreAligned), "CentreAligned", "Center"});
        _enumVerticalTextFormatting = new QtnEnumInfo("VerticalTextFormatting", values);
    }
    return *_enumVerticalTextFormatting;
}

const QtnEnumInfo& CEGUIManager::enumItemListBaseSortMode()
{
    if (!_enumItemListBaseSortMode)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ItemListBase::SortMode::Ascending), "Ascending", "Ascending"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ItemListBase::SortMode::Descending), "Descending", "Descending"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ItemListBase::SortMode::UserSort), "UserSort", "User defined"});
        _enumItemListBaseSortMode = new QtnEnumInfo("ItemListBase::SortMode", values);
    }
    return *_enumItemListBaseSortMode;
}

const QtnEnumInfo& CEGUIManager::enumViewSortMode()
{
    if (!_enumViewSortMode)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ViewSortMode::NoSorting), "NoSorting", "No sorting"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ViewSortMode::Ascending), "Ascending", "Ascending"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ViewSortMode::Descending), "Descending", "Descending"});
        _enumViewSortMode = new QtnEnumInfo("ItemListBase::SortMode", values);
    }
    return *_enumViewSortMode;
}

const QtnEnumInfo& CEGUIManager::enumScrollbarDisplayMode()
{
    if (!_enumScrollbarDisplayMode)
    {
        QVector<QtnEnumValueInfo> values;
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ScrollbarDisplayMode::Shown), "Shown", "Shown"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ScrollbarDisplayMode::Hidden), "Hidden", "Hidden"});
        values.push_back({static_cast<QtnEnumValueType>(CEGUI::ScrollbarDisplayMode::WhenNeeded), "WhenNeeded", "When needed"});
        _enumScrollbarDisplayMode = new QtnEnumInfo("ItemListBase::SortMode", values);
    }
    return *_enumScrollbarDisplayMode;
}
