#include "src/editors/imageset/ImagesetEditor.h"
#include "src/editors/imageset/ImagesetCodeMode.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"
#include "src/ui/MainWindow.h"
#include "qmenu.h"
#include "qdom.h"
#include "qfile.h"
#include "qmessagebox.h"

ImagesetEditor::ImagesetEditor(const QString& filePath)
    : MultiModeEditor(/*imageset_compatibility.manager, */ filePath)
{
    visualMode = new ImagesetVisualMode(*this);
    tabs.addTab(visualMode, "Visual");

    codeMode = new ImagesetCodeMode(*this);
    tabs.addTab(codeMode, "Code");
/*
        # set the toolbar icon size according to the setting and subscribe to it
        self.tbIconSizeEntry = settings.getEntry("global/ui/toolbar_icon_size")
        self.updateToolbarSize(self.tbIconSizeEntry.value)
        self.tbIconSizeCallback = lambda value: self.updateToolbarSize(value)
        self.tbIconSizeEntry.subscribe(self.tbIconSizeCallback)
    def updateToolbarSize(self, size):
        if size < 16:
            size = 16
        self.visual.toolBar.setIconSize(QtCore.QSize(size, size))
*/
}

void ImagesetEditor::initialize()
{
    MultiModeEditor::initialize();

    QDomDocument doc;

    {
        QFile file(_filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            assert(false);
            return;
        }

        const auto fileSize = file.size();
        if (!doc.setContent(&file))
        {
            // Things didn't go smooth
            // 2 reasons for that
            //  * the file is empty
            //  * the contents of the file are invalid
            //
            // In the first case we will silently move along (it is probably just a new file),
            // in the latter we will output a message box informing about the situation

            if (fileSize > 2)
            {
                // The file contains more than just CR LF
                QMessageBox::question(&tabs,
                                      "Can't parse given imageset!",
                                      QString("Parsing '%1' failed, it's most likely not a valid XML file. "
                                      "Constructing empty imageset instead (if you save you will override the invalid data!). "
                                      ).arg(_filePath),
                                      QMessageBox::Ok);
            }

            QDomElement xmlRoot = doc.createElement("Imageset");
            xmlRoot.setAttribute("Name", "");
            xmlRoot.setAttribute("Imagefile", "");
            doc.appendChild(xmlRoot);
        }
    }

    visualMode->loadImagesetEntryFromElement(doc.documentElement());
}

void ImagesetEditor::finalize()
{
    // Unsubscribe from the toolbar icon size setting
/*
        self.tbIconSizeEntry.unsubscribe(self.tbIconSizeCallback)
*/
    MultiModeEditor::finalize();
}

void ImagesetEditor::activate(MainWindow& mainWindow)
{
    MultiModeEditor::activate(mainWindow);
/*
        self.mainWindow.addToolBar(QtCore.Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()

        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.dockWidget)
        self.visual.dockWidget.setVisible(True)
*/

    auto editorMenu = mainWindow.getEditorMenu();
    editorMenu->setTitle("&Imageset");
/*
        self.visual.rebuildEditorMenu(editorMenu)
*/
    editorMenu->setVisible(true);
    editorMenu->setEnabled(tabs.currentWidget() == visualMode);
}

void ImagesetEditor::deactivate(MainWindow& mainWindow)
{
    /*
        self.mainWindow.removeDockWidget(self.visual.dockWidget)
        self.mainWindow.removeToolBar(self.visual.toolBar)
    */
    MultiModeEditor::deactivate(mainWindow);
}

void ImagesetEditor::copy()
{
    if (tabs.currentWidget() == visualMode)
        ; //visualMode->copy();
}

void ImagesetEditor::cut()
{
    if (tabs.currentWidget() == visualMode)
        ; //visualMode->cut();
}

void ImagesetEditor::paste()
{
    if (tabs.currentWidget() == visualMode)
        ; //visualMode->paste();
}

void ImagesetEditor::deleteSelected()
{
    if (tabs.currentWidget() == visualMode)
        ; //visualMode->deleteSelected();
}

void ImagesetEditor::zoomIn()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomIn();
}

void ImagesetEditor::zoomOut()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomOut();
}

void ImagesetEditor::zoomReset()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomReset();
}

void ImagesetEditor::getRawData(QByteArray& outRawData)
{
    // If user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)
    if (tabs.currentWidget() == codeMode)
        codeMode->propagateToVisual();

/*
        rootElement = self.visual.imagesetEntry.saveToElement()
        # we indent to make the resulting files as readable as possible
        xmledit.indent(rootElement)

        self.nativeData = ElementTree.tostring(rootElement, "utf-8")
*/
    //outRawData = string.toUtf8();
}

void ImagesetEditor::createActions(ActionManager& mgr)
{
/*
    cat = actionManager.createCategory(name = "imageset", label = "Imageset Editor")

    cat.createAction(name = "edit_offsets", label = "Edit &Offsets",
                     help_ = "When you select an image definition, a crosshair will appear in it representing it's offset centrepoint.",
                     icon = QtGui.QIcon("icons/imageset_editing/edit_offsets.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Space)).setCheckable(True)

    cat.createAction(name = "cycle_overlapping", label = "Cycle O&verlapping Image Definitions",
                     help_ = "When images definition overlap in such a way that makes it hard/impossible to select the definition you want, this allows you to select on of them and then just cycle until the right one is selected.",
                     icon = QtGui.QIcon("icons/imageset_editing/cycle_overlapping.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Q))

    cat.createAction(name = "create_image", label = "&Create Image Definition",
                     help_ = "Creates a new image definition at the current cursor position, sized 50x50 pixels.",
                     icon = QtGui.QIcon("icons/imageset_editing/create_image.png"))

    cat.createAction(name = "duplicate_image", label = "&Duplicate Image Definition",
                     help_ = "Duplicates selected image definitions.",
                     icon = QtGui.QIcon("icons/imageset_editing/duplicate_image.png"))

    cat.createAction(name = "focus_image_list_filter_box", label = "&Focus Image Definition List Filter Box",
                     help_ = "This allows you to easily press a shortcut and immediately search through image definitions without having to reach for a mouse.",
                     icon = QtGui.QIcon("icons/imageset_editing/focus_image_list_filter_box.png"),
                     defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))
*/
}

void ImagesetEditor::createSettings(Settings& mgr)
{
    auto catImageset = mgr.createCategory("imageset", "Imageset editing");
    auto secVisual = catImageset->createSection("visual", "Visual editing");

    SettingsEntryPtr entry(new SettingsEntry(*secVisual, "overlay_image_labels", true, "Show overlay labels of images",
                                             "Show overlay labels of images.",
                                             "checkbox", false, 1));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "partial_updates", false, "Use partial drawing updates",
                                  "Will use partial 2D updates using accelerated 2D machinery. The performance of this is very "
                                  "dependent on your platform and hardware. MacOSX handles partial updates much better than Linux "
                                  "it seems. If you have a very good GPU, don't tick this.",
                                  "checkbox", true, 2));
    secVisual->addEntry(std::move(entry));
}

//---------------------------------------------------------------------

QString ImagesetEditorFactory::getFileTypesDescription() const
{
    return "Imageset files";
}

QStringList ImagesetEditorFactory::getFileExtensions() const
{
    /*
        extensions = imageset_compatibility.manager.getAllPossibleExtensions()
    */
    return { "imageset" };
}

EditorBasePtr ImagesetEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<ImagesetEditor>(filePath);
}
