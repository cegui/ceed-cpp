#include "src/editors/imageset/ImagesetEditor.h"
#include "src/editors/imageset/ImagesetCodeMode.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"
#include "src/ui/imageset/ImagesetEditorDockWidget.h"
#include "src/ui/imageset/ImagesetEntry.h"
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include "qmenu.h"
#include "qdom.h"
#include "qfile.h"
#include "qmessagebox.h"
#include "qtoolbar.h"

ImagesetEditor::ImagesetEditor(const QString& filePath)
    : MultiModeEditor(/*imageset_compatibility.manager, */ filePath)
{
    visualMode = new ImagesetVisualMode(*this);
    tabs.addTab(visualMode, "Visual");

    codeMode = new ImagesetCodeMode(*this);
    tabs.addTab(codeMode, "Code");
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

void ImagesetEditor::activate(MainWindow& mainWindow)
{
    MultiModeEditor::activate(mainWindow);

    auto toolBar = mainWindow.getToolbar("Imageset");
    mainWindow.addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);
    toolBar->show();

    mainWindow.addDockWidget(Qt::RightDockWidgetArea, visualMode->getDockWidget());
    visualMode->getDockWidget()->setVisible(true);

    auto editorMenu = mainWindow.getEditorMenu();
    editorMenu->setTitle("&Imageset");
    visualMode->rebuildEditorMenu(editorMenu);
    editorMenu->menuAction()->setVisible(true);
    editorMenu->menuAction()->setEnabled(tabs.currentWidget() == visualMode);
}

void ImagesetEditor::deactivate(MainWindow& mainWindow)
{
    mainWindow.removeDockWidget(visualMode->getDockWidget());
    mainWindow.removeToolBar(mainWindow.getToolbar("Imageset"));
    MultiModeEditor::deactivate(mainWindow);
}

void ImagesetEditor::copy()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->copy();
}

void ImagesetEditor::cut()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->cut();
}

void ImagesetEditor::paste()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->paste();
}

void ImagesetEditor::deleteSelected()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->deleteSelectedImageEntries();
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

QString ImagesetEditor::getSourceCode() const
{
    QDomDocument doc;
    auto xmlRoot = doc.createElement("Imageset");
    visualMode->getImagesetEntry()->saveToElement(xmlRoot);
    doc.appendChild(xmlRoot);

    return doc.toString(4);
}

void ImagesetEditor::getRawData(QByteArray& outRawData)
{
    // If user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)
    if (tabs.currentWidget() == codeMode)
        codeMode->propagateToVisual();

    outRawData = getSourceCode().toUtf8();
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

void ImagesetEditor::createActions(Application& app)
{
    app.getOrCreateShortcutSettingsSection("imageset", "Imageset Editor");

    app.registerAction("imageset", "edit_offsets", "Edit &Offsets",
                       "When you select an image definition, a crosshair will appear in it representing it's offset centrepoint.",
                       QIcon(":/icons/imageset_editing/edit_offsets.png"), QKeySequence(Qt::Key_Space), true);

    app.registerAction("imageset", "cycle_overlapping", "Cycle O&verlapping Image Definitions",
                       "When images definition overlap in such a way that makes it hard/impossible to select the definition you want, this allows you to select on of them and then just cycle until the right one is selected.",
                       QIcon(":/icons/imageset_editing/cycle_overlapping.png"), QKeySequence(Qt::Key_Q));

    app.registerAction("imageset", "create_image", "&Create Image Definition",
                       "Creates a new image definition at the current cursor position, sized 50x50 pixels.",
                       QIcon(":/icons/imageset_editing/create_image.png"));

    app.registerAction("imageset", "duplicate_image", "&Duplicate Image Definition",
                       "Duplicates selected image definitions.",
                       QIcon(":/icons/imageset_editing/duplicate_image.png"));

    app.registerAction("imageset", "focus_image_list_filter_box", "&Filter...",
                       "This allows you to easily press a shortcut and immediately search through image definitions without having to reach for a mouse.",
                       QIcon(":/icons/imageset_editing/focus_image_list_filter_box.png"), QKeySequence(QKeySequence::Find));
}

void ImagesetEditor::createToolbar(Application& app)
{
    auto toolBar = app.getMainWindow()->createToolbar("Imageset");
    toolBar->addAction(app.getAction("imageset/create_image"));
    toolBar->addAction(app.getAction("imageset/duplicate_image"));
    toolBar->addSeparator();
    toolBar->addAction(app.getAction("imageset/edit_offsets"));
    toolBar->addAction(app.getAction("imageset/cycle_overlapping"));
    toolBar->setVisible(false);
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
