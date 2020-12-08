#include "src/cegui/CEGUIProject.h"
#include "src/cegui/CEGUIProjectItem.h"
#include "src/Application.h"
#include <qdir.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qmessagebox.h>

const QString CEGUIProject::EditorEmbeddedCEGUIVersion("1.0");
const QStringList CEGUIProject::CEGUIVersions = { "0.6", "0.7", "0.8", "1.0" };

CEGUIProject::CEGUIProject()
    : CEGUIVersion(EditorEmbeddedCEGUIVersion)
    , baseDirectory("./")
    , imagesetsPath("./imagesets")
    , fontsPath("./fonts")
    , looknfeelsPath("./looknfeel")
    , schemesPath("./schemes")
    , layoutsPath("./layouts")
    , xmlSchemasPath("./xml_schemas")
    , defaultResolution(1280, 720) // 720p seems like a decent default nowadays, 16:9
{
    setHorizontalHeaderLabels({ "Name" });

    // NB: we must not delete it, Qt does this for us
    setItemPrototype(new CEGUIProjectItem(this));
}

CEGUIProject::~CEGUIProject()
{
}

Qt::DropActions CEGUIProject::supportedDragActions() const
{
    return Qt::MoveAction;
}

// Loads XML project file from given path (preferably absolute path)
bool CEGUIProject::loadFromFile(const QString& fileName)
{
    QDomDocument doc;

    // Open, read & close file. We will work with a DOM document.
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        if (!doc.setContent(&file))
            return false;
    }

    filePath = fileName;

    auto xmlRoot = doc.documentElement();
    setDefaultResolution(xmlRoot.attribute("CEGUIDefaultResolution", "1280x720"));
    CEGUIVersion = xmlRoot.attribute("CEGUIVersion", EditorEmbeddedCEGUIVersion);

    baseDirectory = QDir::cleanPath(xmlRoot.attribute("baseDirectory", "./"));
    imagesetsPath = QDir::cleanPath(xmlRoot.attribute("imagesetsPath", "./imagesets"));
    fontsPath = QDir::cleanPath(xmlRoot.attribute("fontsPath", "./fonts"));
    looknfeelsPath = QDir::cleanPath(xmlRoot.attribute("looknfeelsPath", "./looknfeel"));
    schemesPath = QDir::cleanPath(xmlRoot.attribute("schemesPath", "./schemes"));
    layoutsPath = QDir::cleanPath(xmlRoot.attribute("layoutsPath", "./layouts"));
    xmlSchemasPath = QDir::cleanPath(xmlRoot.attribute("xmlSchemasPath", "./xml_schemas"));
    //???animations?

    //!!!TODO: scan & add files from project directories!
    // New project created in an existing folder structure must auto-add files.
    auto xmlItem = xmlRoot.firstChildElement("Items").firstChildElement("Item");
    while (!xmlItem.isNull())
    {
        CEGUIProjectItem* itemPtr = new CEGUIProjectItem(this);
        itemPtr->loadFromElement(xmlItem);
        appendRow(itemPtr);

        xmlItem = xmlItem.nextSiblingElement("Item");
    }

    changed = false;

    return true;
}

// Saves in "CEED Project 1" format by default
bool CEGUIProject::save(const QString& newFilePath)
{
    if (newFilePath.isEmpty())
    {
        // "save" vs "save as"
        changed = false;
    }
    else
    {
        // Set the project's file path to newPath so that if you press save next time it will save to the new path
        // (This is what is expected from applications in general I think)
        filePath = newFilePath;
    }

    QDomDocument doc;

    // Write header

    auto xmlRoot = doc.createElement("Project");
    xmlRoot.setAttribute("version", "CEED Project 1");
    xmlRoot.setAttribute("CEGUIVersion", CEGUIVersion);
    xmlRoot.setAttribute("CEGUIDefaultResolution", getDefaultResolutionString());
    xmlRoot.setAttribute("baseDirectory", QDir::cleanPath(baseDirectory));
    xmlRoot.setAttribute("imagesetsPath", QDir::cleanPath(imagesetsPath));
    xmlRoot.setAttribute("fontsPath", QDir::cleanPath(fontsPath));
    xmlRoot.setAttribute("looknfeelsPath", QDir::cleanPath(looknfeelsPath));
    xmlRoot.setAttribute("schemesPath", QDir::cleanPath(schemesPath));
    xmlRoot.setAttribute("layoutsPath", QDir::cleanPath(layoutsPath));
    xmlRoot.setAttribute("xmlSchemasPath", QDir::cleanPath(xmlSchemasPath));
    //???animations?

    // Write project item tree

    QDomElement xmlItems = doc.createElement("Items");

    for (int i = 0; i < rowCount(); ++i)
    {
        QDomElement xmlChild = doc.createElement("Item");
        auto itemPtr = static_cast<CEGUIProjectItem*>(item(i));
        if (itemPtr && itemPtr->saveToElement(xmlChild))
            xmlItems.appendChild(xmlChild);
    }

    xmlRoot.appendChild(xmlItems);
    doc.appendChild(xmlRoot);

    // If the file already exists, rename it to avoid data losing due to save error
    const bool tmpUsed = QFileInfo(filePath).exists();
    if (tmpUsed)
    {
        QFile prevFile(filePath);
        if (!prevFile.rename(filePath + ".bak"))
        {
            QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                                  "Error saving project!",
                                  "CEED encountered an error trying to save the project file " + filePath);
            return false;
        }
    }

    // Save to file
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        doc.save(stream, 4);
        file.close();
        if (tmpUsed) QFile(filePath + ".bak").remove();
        return true;
    }

    // Something went wrong, show error and restore backup
    QMessageBox::critical(qobject_cast<Application*>(qApp)->getMainWindow(),
                          "Error saving project!",
                          "CEED encountered an error trying to save the project file " + filePath);

    if (tmpUsed)
    {
        QFile(filePath).remove();
        QFile(filePath + ".bak").rename(filePath);
    }

    return false;
}

void CEGUIProject::unload()
{
    // Was empty in the original CEED
}

// Performs base directory and resource directories sanity check, raises IOError in case of a failure
bool CEGUIProject::checkAllDirectories() const
{
    // Check the base directory
    if (!QFileInfo(getAbsolutePathOf("")).isDir())
    {
        // raise IOError("Base directory '%s' isn't a directory or isn't accessible." % (self.getAbsolutePathOf("")))
        return false;
    }

    const QString categories[] = { "imagesets", "fonts", "looknfeels", "schemes", "layouts", "xml_schemas" };
    for (const auto& resourceCategory : categories)
    {
        QString directoryPath = getResourceFilePath("", resourceCategory);
        if (!QFileInfo(directoryPath).isDir())
        {
            // raise IOError("Resource directory '%s' for resources of type '%s' isn't a directory or isn't accessible" % (directoryPath, resourceCategory))
            return false;
        }
    }

    return true;
}

QString CEGUIProject::getName() const
{
    return QFileInfo(filePath).baseName();
}

// Converts project relative paths to absolute paths
QString CEGUIProject::getAbsolutePathOf(const QString& relPath) const
{
    QDir absBaseDir(QFileInfo(filePath).dir().absoluteFilePath(baseDirectory));
    return QDir::cleanPath(absBaseDir.absoluteFilePath(relPath));
}

QString CEGUIProject::getRelativePathOf(const QString& absPath) const
{
    QDir absBaseDir(QFileInfo(filePath).dir().absoluteFilePath(baseDirectory));
    return QDir::cleanPath(absBaseDir.relativeFilePath(absPath));
}

QString CEGUIProject::getResourceFilePath(const QString& fileName, const QString& resourceGroup) const
{
    // FIXME: The whole resource provider wrapping should be done proper, see http://www.cegui.org.uk/mantis/view.php?id=552
    QString folder;
    if (resourceGroup == "imagesets")
        folder = imagesetsPath;
    else if (resourceGroup == "fonts")
        folder = fontsPath;
    else if (resourceGroup == "looknfeels")
        folder = looknfeelsPath;
    else if (resourceGroup == "schemes")
        folder = schemesPath;
    else if (resourceGroup == "layouts")
        folder = layoutsPath;
    else if (resourceGroup == "xml_schemas")
        folder = xmlSchemasPath;
    else
    {
        //???throw?
        return QString();
    }

    return getAbsolutePathOf(QDir(folder).filePath(fileName));
}

// Checks whether given absolute path is referenced by any File item in the project
bool CEGUIProject::referencesFilePath(const QString& filePath) const
{
    for (int i = 0; i < rowCount(); ++i)
    {
        auto itemPtr = static_cast<CEGUIProjectItem*>(item(i));
        if (itemPtr->referencesFilePath(filePath))
            return true;
    }

    return false;
}

void CEGUIProject::setDefaultResolution(const QString& string)
{
    auto sepPos = string.indexOf('x');
    if (sepPos < 0) return;
    defaultResolution.setWidth(string.leftRef(sepPos).toInt());
    defaultResolution.setHeight(string.midRef(sepPos + 1).toInt());
}

QString CEGUIProject::getDefaultResolutionString() const
{
    return QString("%1x%2").arg(defaultResolution.width()).arg(defaultResolution.height());
}
