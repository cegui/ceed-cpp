#include "src/proj/CEGUIProject.h"
#include "qfile.h"
#include "qdom.h"

CEGUIProject::CEGUIProject()
{

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

    auto xmlRoot = doc.documentElement();
    defaultResolution = xmlRoot.attribute("CEGUIDefaultResolution", "1280x720");
    /*
        self.baseDirectory = os.path.normpath(root.get("baseDirectory", "./"))
        self.CEGUIVersion = root.get("CEGUIVersion", compatibility.EditorEmbeddedCEGUIVersion)

        self.imagesetsPath = os.path.normpath(root.get("imagesetsPath", "./imagesets"))
        self.fontsPath = os.path.normpath(root.get("fontsPath", "./fonts"))
        self.looknfeelsPath = os.path.normpath(root.get("looknfeelsPath", "./looknfeel"))
        self.schemesPath = os.path.normpath(root.get("schemesPath", "./schemes"))
        self.layoutsPath = os.path.normpath(root.get("layoutsPath", "./layouts"))
        self.xmlSchemasPath = os.path.normpath(root.get("xmlSchemasPath", "./xml_schemas"))

        items = root.find("Items")

        for itemElement in items.findall("Item"):
            item = Item.loadFromElement(self, itemElement)
            self.appendRow(item)

        self.changed = False
        self.projectFilePath = path
    */
    return true;
}
