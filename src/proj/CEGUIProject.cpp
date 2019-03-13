#include "src/proj/CEGUIProject.h"
#include "src/proj/CEGUIProjectItem.h"
#include "qfile.h"
#include "qdir.h"
#include "qdom.h"
#include "qtextstream.h"

//!!!EditorEmbeddedCEGUIVersion must be a version string from the current CEGUI system!
static const QString EditorEmbeddedCEGUIVersion("1.0");

CEGUIProject::CEGUIProject()
    : defaultResolution("1280x720") // 720p seems like a decent default nowadays, 16:9
    , CEGUIVersion(EditorEmbeddedCEGUIVersion)
    , baseDirectory("./")
    , imagesetsPath("./imagesets")
    , fontsPath("./fonts")
    , looknfeelsPath("./looknfeel")
    , schemesPath("./schemes")
    , layoutsPath("./layouts")
    , xmlSchemasPath("./xml_schemas")
{
    setHorizontalHeaderLabels({ "Name" });

    itemPrototype = new CEGUIProjectItem(this);
    setItemPrototype(itemPrototype);
/*
    pmappings = set(["mappings/Base.pmappings"])
    self.propertyMap = propertymapping.PropertyMap.fromFiles([os.path.abspath(path) for path in pmappings])
*/
}

CEGUIProject::~CEGUIProject()
{
    if (itemPrototype)
        delete itemPrototype;
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
    defaultResolution = xmlRoot.attribute("CEGUIDefaultResolution", "1280x720");
    CEGUIVersion = xmlRoot.attribute("CEGUIVersion", EditorEmbeddedCEGUIVersion);

    baseDirectory = QDir::cleanPath(xmlRoot.attribute("baseDirectory", "./"));
    imagesetsPath = QDir::cleanPath(xmlRoot.attribute("imagesetsPath", "./imagesets"));
    fontsPath = QDir::cleanPath(xmlRoot.attribute("fontsPath", "./fonts"));
    looknfeelsPath = QDir::cleanPath(xmlRoot.attribute("looknfeelsPath", "./looknfeel"));
    schemesPath = QDir::cleanPath(xmlRoot.attribute("schemesPath", "./schemes"));
    layoutsPath = QDir::cleanPath(xmlRoot.attribute("layoutsPath", "./layouts"));
    xmlSchemasPath = QDir::cleanPath(xmlRoot.attribute("xmlSchemasPath", "./xml_schemas"));
    //???animations?

    changed = false;

    //!!!TODO: scan & add files from project directories!
    auto xmlItem = xmlRoot.firstChildElement("Items").firstChildElement("Item");
    while (!xmlItem.isNull())
    {
        /*
        item = Item.loadFromElement(self, itemElement)
        self.appendRow(item)
        */

        xmlItem = xmlItem.nextSiblingElement("Item");
    }

    return true;
}

// Saves in "CEED Project 1" format by default
bool CEGUIProject::save(const QString& fileName)
{
    QString path;
    if (fileName.isEmpty())
    {
        // "save" vs "save as"
        path = filePath;
        changed = false;
    }
    else path = fileName;

    QDomDocument doc;
    auto xmlRoot = doc.documentElement();
    xmlRoot.setTagName("Project");
    xmlRoot.setAttribute("version", "CEED Project 1");
    xmlRoot.setAttribute("CEGUIVersion", CEGUIVersion);
    xmlRoot.setAttribute("CEGUIDefaultResolution", defaultResolution);
    xmlRoot.setAttribute("baseDirectory", QDir::cleanPath(baseDirectory));
    xmlRoot.setAttribute("imagesetsPath", QDir::cleanPath(imagesetsPath));
    xmlRoot.setAttribute("fontsPath", QDir::cleanPath(fontsPath));
    xmlRoot.setAttribute("looknfeelsPath", QDir::cleanPath(looknfeelsPath));
    xmlRoot.setAttribute("schemesPath", QDir::cleanPath(schemesPath));
    xmlRoot.setAttribute("layoutsPath", QDir::cleanPath(layoutsPath));
    xmlRoot.setAttribute("xmlSchemasPath", QDir::cleanPath(xmlSchemasPath));
    //???animations?

    QDomElement xmlItems = doc.createElement("Items");
/*
    i = 0
    while i < self.rowCount():
        items.append(self.item(i).saveToElement())
        i = i + 1
*/

    xmlRoot.appendChild(xmlItems);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug("CEGUIProject::save() > can't open the file for writing: "); // + path);
        return false;
    }

    QTextStream stream(&file);
    doc.save(stream, 4);
    return true;
}

void CEGUIProject::unload()
{
    //
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
        auto itemPtr = item(i);
        if (itemPtr->referencesFilePath(filePath))
            return true;
    }

    return false;
}
