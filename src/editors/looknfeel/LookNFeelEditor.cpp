#include "src/editors/looknfeel/LookNFeelEditor.h"
#include "src/editors/looknfeel/LookNFeelVisualMode.h"
#include "src/editors/looknfeel/LookNFeelCodeMode.h"
#include "src/editors/looknfeel/LookNFeelPreviewMode.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/DismissableMessage.h"
#include "src/util/Settings.h"
#include <CEGUI/falagard/WidgetLookManager.h>
#include "qmessagebox.h"

LookNFeelEditor::LookNFeelEditor(const QString& filePath)
    : MultiModeEditor(/*looknfeel_compatibility.manager, */ filePath)
{
    DismissableMessage::warning(nullptr, "LookNFeel Editor is experimental!",
                                "This part of CEED is not considered to be ready for "
                                "production. You have been warned. If everything "
                                "breaks you get to keep the pieces!",
                                "looknfeel_editor_experimental");

    _editorIDString = CEGUIManager::getEditorIDStringPrefix() + QString::number(reinterpret_cast<size_t>(this));

    visualMode = new LookNFeelVisualMode(*this);
    tabs.addTab(visualMode, "Visual");

    codeMode = new LookNFeelCodeMode(*this);
    tabs.addTab(codeMode, "Code");

    // Look n' Feel Previewer is not actually an edit mode, you can't edit the Look n' Feel from it,
    // however for everything to work smoothly we do push edit mode changes to it to the
    // undo stack.
    //
    // TODO: This could be improved at least a little bit if 2 consecutive edit mode changes
    //       looked like this: A->Preview, Preview->C.  We could simply turn this into A->C,
    //       and if A = C it would eat the undo command entirely.
    auto previewer = new LookNFeelPreviewMode(*this);
    tabs.addTab(previewer, "Live Preview");

/*
        // set the toolbar icon size according to the setting and subscribe to it
        self.tbIconSizeEntry = settings.getEntry("global/ui/toolbar_icon_size")
        self.updateToolbarSize(self.tbIconSizeEntry.value)
        self.tbIconSizeCallback = lambda value: self.updateToolbarSize(value)
        self.tbIconSizeEntry.subscribe(self.tbIconSizeCallback)
*/
}

void LookNFeelEditor::initialize()
{
    MultiModeEditor::initialize();

    try
    {
        /*
            self.mapAndLoadLookNFeelFileString(self.nativeData)
        */
        /*
        // FIXME: open manually and load from string?
        auto layoutFileName = QDir(CEGUIManager::Instance().getCurrentProject()->getResourceFilePath("", "layouts")).relativeFilePath(_filePath);
        CEGUI::Window* root = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(CEGUIUtils::qStringToString(layoutFileName));
        */
        visualMode->initialize();
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(nullptr, "Exception", e.what());
    }
}

void LookNFeelEditor::mapAndLoadLookNFeelFileString(const QString& lookNFeelXML)
{
    // When we are loading a Look n' Feel file we want to load it into CEED in a way it doesn't collide with other LNF definitions stored into CEGUI.
    // To prevent name collisions and also to prevent live-editing of WidgetLooks that are used somewhere in a layout editor or other look n' feel editor simultaneously,
    // we will map the names that we load from a Look n' Feel file in a way that they are unique. We achieve this by editing the WidgetLook names inside the string we loaded
    // from the .looknfeel file, so that the LNF Editor instance's python ID will be prepended to the name. ( e.g.: Vanilla/Button will turn into 18273822/Vanilla/Button )
    // Each Editor is associated with only one LNF file so that this will in effect also guarantee that the WidgetLooks inside the CEGUI system will be uniquely named
    // for each file

    if (lookNFeelXML.isEmpty()) return;

/*
        //Mapping all occuring references
        modifiedLookNFeelString = self.mapWidgetLookReferences(lookNFeelAsXMLString)

        // We remove all WidgetLook mappings
        self.removeOwnedWidgetLookFalagardMappings()

        // We erase all widgetLooks
        self.destroyOwnedWidgetLooks()

        try:
            // Parsing the resulting Look n' Feel
            PyCEGUI.WidgetLookManager.getSingleton().parseLookNFeelSpecificationFromString(modifiedLookNFeelString)
        except:
            // We retrieve a list of all newly loaded WidgetLook names (as tuples of original and new name) that we just mapped for this editor
            self.refreshWidgetLookNameMappingTuples()
            // We erase all widgetLooks
            self.destroyOwnedWidgetLooks()
            // Refreshing the combobox
            self.visual.lookNFeelWidgetLookSelectorWidget.populateWidgetLookComboBox(self.nameMappingsOfOwnedWidgetLooks)
            // We raise the error again so we can process it in the try-block outside the function
            raise

        // We retrieve a list of all WidgetLook names (as tuples of original and new name) that we just mapped for this editor
        self.refreshWidgetLookNameMappingTuples()
        // We look for falagard mappings and add them
        self.addMappedWidgetLookFalagardMappings()
        // Refreshing the combobox
        self.visual.lookNFeelWidgetLookSelectorWidget.populateWidgetLookComboBox(self.nameMappingsOfOwnedWidgetLooks)
*/
}

// Fills a CEGUI::StringSet with all (mapped) names of WidgetLookFeels that the file is associated with according to the editor
void LookNFeelEditor::getWidgetLookFeelNames(std::unordered_set<CEGUI::String>& out) const
{
    // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
    for (const auto& pair : _nameMappingsOfOwnedWidgetLooks)
        out.insert(pair.second);
}

// Unmaps all occurances of mapped WidgetLookFeel name references in an XML string by removing the prepended editor ID number
QString LookNFeelEditor::unmapWidgetLookReferences(const CEGUI::String& lookNFeelString)
{
    /*
            // Modifying the string using regex
            QString regexPattern = "name=\"" + self.editorIDString + "/";
            QString replaceString = "name=\"";
            modifiedLookNFeelString = re.sub(regexPattern, replaceString, lookNFeelString);

            regexPattern = "look=\"" + self.editorIDString + "/";
            replaceString = "look=\"";
            modifiedLookNFeelString = re.sub(regexPattern, replaceString, modifiedLookNFeelString);

            return modifiedLookNFeelString;
    */

    assert(false);
    return "";
}

QString LookNFeelEditor::getFileTypesDescription() const
{
    return LookNFeelEditorFactory::lnfFileTypesDescription();
}

QStringList LookNFeelEditor::getFileExtensions() const
{
    return LookNFeelEditorFactory::lnfFileExtensions();
}

void LookNFeelEditor::getRawData(QByteArray& outRawData)
{
    // If user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)
    if (tabs.currentWidget() == codeMode)
        codeMode->propagateToVisual();

    // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
    std::unordered_set<CEGUI::String> nameSet;
    getWidgetLookFeelNames(nameSet);

    // We parse all WidgetLookFeels as XML to a string
    auto lookAndFeelString = CEGUI::WidgetLookManager::getSingleton().getWidgetLookSetAsString(nameSet);
    outRawData = CEGUIUtils::stringToQString(lookAndFeelString).toUtf8();
}

/*
    @staticmethod
    def unmapMappedNameIntoOriginalParts(mappedName):
        """
        Returns the original WidgetLookFeel name and the editorID, based on a mapped name
        :param mappedName: str
        :return: str, str
        """
        mappedNameSplitResult = mappedName.split('/', 1)

        if len(mappedNameSplitResult) != 2:
            raise Exception('Failed to split the mapped name')

        return mappedNameSplitResult[1], mappedNameSplitResult[0]

    def tryUpdatingWidgetLookFeel(self, sourceCode):
        """
        Tries to parse a LNF source code content
        :param sourceCode:
        :return:
        """

        loadingSuccessful = True
        try:
            self.mapAndLoadLookNFeelFileString(sourceCode)
        except:
            self.mapAndLoadLookNFeelFileString(self.nativeData)
            loadingSuccessful = False

        return loadingSuccessful

    def mapWidgetLookReferences(self, lookNFeelString):
        """
        Maps all occurances of WidgetLookFeel name references in an XML string to a new name based by prepending the editor's ID number
        :type lookNFeelString: str
        :return: str
        """
        // Modifying the string using regex
        regexPattern = "<\s*WidgetLook\sname\s*=\s*\""
        replaceString = "<WidgetLook name=\"" + self.editorIDString + "/"
        modifiedLookNFeelString = re.sub(regexPattern, replaceString, lookNFeelString)

        regexPattern = "look\s*=\s*\""
        replaceString = "look=\"" + self.editorIDString + "/"
        modifiedLookNFeelString = re.sub(regexPattern, replaceString, modifiedLookNFeelString)

        return modifiedLookNFeelString

    def destroyOwnedWidgetLooks(self):
        for nameTuple in self.nameMappingsOfOwnedWidgetLooks:
            PyCEGUI.WidgetLookManager.getSingleton().eraseWidgetLook(nameTuple[1])

        // We refresh the WidgetLook names
        self.refreshWidgetLookNameMappingTuples()

    def refreshWidgetLookNameMappingTuples(self):
        // Delete all previous entries
        del self.nameMappingsOfOwnedWidgetLooks[:]

        // Returns an array containing tuples of the original WidgetLook name and the mapped one
        self.nameMappingsOfOwnedWidgetLooks = []

        widgetLookMap = PyCEGUI.WidgetLookManager.getSingleton().getWidgetLookPointerMap()

        for widgetLookEntry in widgetLookMap:
            widgetLookEditModeName = widgetLookEntry.key
            widgetLookOriginalName, widgetLookEditorID = self.unmapMappedNameIntoOriginalParts(widgetLookEditModeName)

            if widgetLookEditorID == self.editorIDString:
                widgetLookNameTuple = (widgetLookOriginalName, widgetLookEditModeName)
                self.nameMappingsOfOwnedWidgetLooks.append(widgetLookNameTuple)

        self.nameMappingsOfOwnedWidgetLooks.sort()

    def addMappedWidgetLookFalagardMappings(self):
        // We have to "guess" at least one FalagardWindowMapping - we have to keep in mind that there could theoretically be multiple window mappings for one WidgetLook -  ( which
        // contains a targetType and a renderer ) for our WidgetLook so we can display it.
        // If the user has already loaded .scheme files then we can use the WindowFactoryManager for this purpose:
        for nameTuple in self.nameMappingsOfOwnedWidgetLooks:

            falagardMappingIter = PyCEGUI.WindowFactoryManager.getSingleton().getFalagardMappingIterator()
            while not falagardMappingIter.isAtEnd():

                falagardMapping = falagardMappingIter.getCurrentValue()
                if falagardMapping.d_lookName == nameTuple[0]:
                    PyCEGUI.WindowFactoryManager.getSingleton().addFalagardWindowMapping(nameTuple[1], falagardMapping.d_baseType, nameTuple[1],
                                                                                         falagardMapping.d_rendererType)
                falagardMappingIter.next()

    def removeOwnedWidgetLookFalagardMappings(self):
        // Removes all FalagardMappings we previously added
        for nameTuple in self.nameMappingsOfOwnedWidgetLooks:
            PyCEGUI.WindowFactoryManager.getSingleton().removeFalagardWindowMapping(nameTuple[1])

    def finalise(self):
        super(LookNFeelTabbedEditor, self).finalise()

    def destroy(self):
        self.visual.destroy()

        // Remove all FalagardMappings we added
        self.removeOwnedWidgetLookFalagardMappings()

        // Erase all mapped WidgetLooks we added
        self.destroyOwnedWidgetLooks()

        // unsubscribe from the toolbar icon size setting
        self.tbIconSizeEntry.unsubscribe(self.tbIconSizeCallback)

        super(LookNFeelTabbedEditor, self).destroy()

    def rebuildEditorMenu(self, editorMenu):
        editorMenu.setTitle("&Look and Feel")
        self.visual.rebuildEditorMenu(editorMenu)

        return True, self.currentWidget() == self.visual

    def activate(self):
        super(LookNFeelTabbedEditor, self).activate()

        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.lookNFeelWidgetLookSelectorWidget)
        self.visual.lookNFeelWidgetLookSelectorWidget.setVisible(True)

        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.lookNFeelHierarchyDockWidget)
        self.visual.lookNFeelHierarchyDockWidget.setVisible(True)

        self.mainWindow.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.visual.falagardElementEditorDockWidget)
        self.visual.falagardElementEditorDockWidget.setVisible(True)

        self.mainWindow.addToolBar(QtCore.Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()

        self.visual.lookNFeelWidgetLookSelectorWidget.setFileNameLabel()

    def updateToolbarSize(self, size):
        if size < 16:
            size = 16
        self.visual.toolBar.setIconSize(QtCore.QSize(size, size))

    def deactivate(self):
        self.mainWindow.removeDockWidget(self.visual.lookNFeelHierarchyDockWidget)
        self.mainWindow.removeDockWidget(self.visual.lookNFeelWidgetLookSelectorWidget)
        self.mainWindow.removeDockWidget(self.visual.falagardElementEditorDockWidget)

        self.mainWindow.removeToolBar(self.visual.toolBar)

        super(LookNFeelTabbedEditor, self).deactivate()

    def performCut(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCut()

        return False

    def performCopy(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCopy()

        return False

    def performPaste(self):
        if self.currentWidget() is self.visual:
            return self.visual.performPaste()

        return False

    def performDelete(self):
        if self.currentWidget() is self.visual:
            return self.visual.performDelete()

        return False

    def zoomIn(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomIn()

    def zoomOut(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOut()

    def zoomReset(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOriginal()
*/

void LookNFeelEditor::createSettings(Settings& mgr)
{
    /*
    category = settings.createCategory(name = "looknfeel", label = "Look n' Feel editing")

    visual = category.createSection(name = "visual", label = "Visual editing")

    # FIXME: Only applies to newly switched to visual modes!
    visual.createEntry(name = "continuous_rendering", type_ = bool, label = "Continuous rendering",
                       help_ = "Check this if you are experiencing redraw issues (your skin contains an idle animation or such).\nOnly applies to newly switched to visual modes so switch to Code mode or back or restart the application for this to take effect.",
                       defaultValue = False, widgetHint = "checkbox",
                       sortingWeight = -1)

    visual.createEntry(name = "prevent_manipulator_overlap", type_ = bool, label = "Prevent manipulator overlap",
                       help_ = "Only enable if you have a very fast computer and only edit small Look n' Feels. Very performance intensive!",
                       defaultValue = False, widgetHint = "checkbox",
                       sortingWeight = 0)

    visual.createEntry(name = "normal_outline", type_ = QtGui.QPen, label = "Normal outline",
                       help_ = "Pen for normal outline.",
                       defaultValue = QtGui.QPen(QtGui.QColor(255, 255, 0, 255)), widgetHint = "pen",
                       sortingWeight = 1)

    visual.createEntry(name = "hover_outline", type_ = QtGui.QPen, label = "Hover outline",
                       help_ = "Pen for hover outline.",
                       defaultValue = QtGui.QPen(QtGui.QColor(0, 255, 255, 255)), widgetHint = "pen",
                       sortingWeight = 2)

    visual.createEntry(name = "resizing_outline", type_ = QtGui.QPen, label = "Outline while resizing",
                       help_ = "Pen for resizing outline.",
                       defaultValue = QtGui.QPen(QtGui.QColor(255, 0, 255, 255)), widgetHint = "pen",
                       sortingWeight = 3)

    visual.createEntry(name = "moving_outline", type_ = QtGui.QPen, label = "Outline while moving",
                       help_ = "Pen for moving outline.",
                       defaultValue = QtGui.QPen(QtGui.QColor(255, 0, 255, 255)), widgetHint = "pen",
                       sortingWeight = 4)

    visual.createEntry(name = "snap_grid_x", type_ = float, label = "Snap grid cell width (X)",
                       help_ = "Snap grid X metric.",
                       defaultValue = 5, widgetHint = "float",
                       sortingWeight = 5)

    visual.createEntry(name = "snap_grid_y", type_ = float, label = "Snap grid cell height (Y)",
                       help_ = "Snap grid Y metric.",
                       defaultValue = 5, widgetHint = "float",
                       sortingWeight = 6)

    visual.createEntry(name = "snap_grid_point_colour", type_ = QtGui.QColor, label = "Snap grid point colour",
                       help_ = "Color of snap grid points.",
                       defaultValue = QtGui.QColor(255, 255, 255, 192), widgetHint = "colour",
                       sortingWeight = 7)

    visual.createEntry(name = "snap_grid_point_shadow_colour", type_ = QtGui.QColor, label = "Snap grid point shadow colour",
                       help_ = "Color of snap grid points (shadows).",
                       defaultValue = QtGui.QColor(64, 64, 64, 192), widgetHint = "colour",
                       sortingWeight = 8)

    # TODO: Full restart is not actually needed, just a refresh on all Look n' Feel visual editing modes
    visual.createEntry(name = "hide_deadend_autowidgets", type_ = bool, label = "Hide deadend auto widgets",
                       help_ = "Should auto widgets with no non-auto widgets descendants be hidden in the widget hierarchy?",
                       defaultValue = True, widgetHint = "checkbox",
                       sortingWeight = 9, changeRequiresRestart = True)

    # FIXME: Only applies to newly refreshed visual modes!
    visual.createEntry(name = "auto_widgets_selectable", type_ = bool, label = "Make auto widgets selectable",
                       help_ = "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to deal with them at all. Only for EXPERT use! Will make CEED crash in cases where you don't know what you are doing!",
                       defaultValue = False, widgetHint = "checkbox",
                       sortingWeight = 9)

    # FIXME: Only applies to newly refreshed visual modes!
    visual.createEntry(name = "auto_widgets_show_outline", type_ = bool, label = "Show outline of auto widgets",
                       help_ = "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to deal with them at all. Only use if you know what you are doing! This might clutter the interface a lot.",
                       defaultValue = False, widgetHint = "checkbox",
                       sortingWeight = 10)
    */

    // SAMPLE:

    /*
    auto catImageset = mgr.createCategory("layout", "Layout editing");
    auto secVisual = catImageset->createSection("visual", "Visual editing");

    // FIXME: Only applies to newly switched to visual modes!
    SettingsEntryPtr entry(new SettingsEntry(*secVisual, "continuous_rendering", false, "Continuous rendering",
                                             "Check this if you are experiencing redraw issues (your skin contains an idle animation or such)."
                                             "\nOnly applies to newly switched to visual modes so switch to Code mode or back or restart the application for this to take effect.",
                                             "checkbox", false, -1));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "prevent_manipulator_overlap", false, "Prevent manipulator overlap",
                                  "Only enable if you have a very fast computer and only edit small layouts. Very performance intensive!",
                                  "checkbox", false, 0));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "normal_outline", QPen(QColor(255, 255, 0, 255)), "Normal outline",
                                  "Pen for normal outline.",
                                  "pen", false, 1));
    secVisual->addEntry(std::move(entry));
*/
}
//---------------------------------------------------------------------

QString LookNFeelEditorFactory::lnfFileTypesDescription()
{
    return "Look and feel";
}

QStringList LookNFeelEditorFactory::lnfFileExtensions()
{
    /*
        extensions = looknfeel_compatibility.manager.getAllPossibleExtensions()
    */
    return { "looknfeel" };
}

EditorBasePtr LookNFeelEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<LookNFeelEditor>(filePath);
}
