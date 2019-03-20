#include "src/proj/CEGUIProjectItem.h"
#include "src/proj/CEGUIProject.h"
#include "qdir.h"

CEGUIProjectItem::Type CEGUIProjectItem::getItemType(const QModelIndex& index)
{
    return static_cast<Type>(index.data(Qt::UserRole + 1).toInt());
}

bool CEGUIProjectItem::isFile(const QModelIndex& index)
{
    return getItemType(index) == Type::File;
}

bool CEGUIProjectItem::isFolder(const QModelIndex& index)
{
    return getItemType(index) == Type::Folder;
}

CEGUIProjectItem::CEGUIProjectItem(CEGUIProject* project)
    : _project(project)
{
    setType(Type::Unknown);
    project->setModified();
}

CEGUIProjectItem::~CEGUIProjectItem()
{
}

QStandardItem* CEGUIProjectItem::clone() const
{
    auto type = getType();

    CEGUIProjectItem* ret = new CEGUIProjectItem(_project);
    ret->setType(type);

    if (type == Type::File || type == Type::Folder)
        ret->setPath(getPath());

    return ret;
}

void CEGUIProjectItem::loadFromElement(const QDomElement& xml)
{
    assert(_project);

    QString typeString = xml.attribute("type");
    if (typeString == "file")
    {
        setType(Type::File);
        setPath(QDir::cleanPath(xml.attribute("path")));
    }
    else if (typeString == "folder")
    {
        setType(Type::Folder);
        setPath(xml.attribute("name"));

        auto xmlItem = xml.firstChildElement("Item");
        while (!xmlItem.isNull())
        {
            CEGUIProjectItem* itemPtr = new CEGUIProjectItem(_project);
            itemPtr->loadFromElement(xmlItem);
            appendRow(itemPtr);

            xmlItem = xmlItem.nextSiblingElement("Item");
        }
    }
    else
    {
        // TODO: error "Unknown item type '%s'"
        assert(false);
    }
}

bool CEGUIProjectItem::saveToElement(QDomElement& xml)
{
    xml.setTagName("Item");

    auto type = getType();
    if (type == Type::File)
    {
        xml.setAttribute("type", "file");
        xml.setAttribute("path", QDir::cleanPath(getPath()));
        return true;
    }
    else if (type == Type::Folder)
    {
        xml.setAttribute("type", "folder");
        xml.setAttribute("name", getPath());

        for (int i = 0; i < rowCount(); ++i)
        {
            QDomElement xmlChild = xml.ownerDocument().createElement("Item");
            auto itemPtr = static_cast<CEGUIProjectItem*>(child(i));
            if (itemPtr && itemPtr->saveToElement(xmlChild))
                xml.appendChild(xmlChild);
        }

        return true;
    }

    return false;
}

void CEGUIProjectItem::setType(CEGUIProjectItem::Type type)
{
    setData(static_cast<int>(type), Qt::UserRole + 1);

    // We can drag files but we can't drop anything to them
    // We can drag folders and drop other items to them
    setDragEnabled(type == Type::File || type == Type::Folder);
    setDropEnabled(type == Type::Folder);
}

CEGUIProjectItem::Type CEGUIProjectItem::getType() const
{
    return static_cast<Type>(data(Qt::UserRole + 1).toInt());
}

void CEGUIProjectItem::setPath(const QString& path)
{
    switch (getType())
    {
        case Type::File:
        {
            QFileInfo pathInfo(path);
            setText(pathInfo.fileName());

            QString fileType = "unknown";
            QString extension = pathInfo.completeSuffix();

            if (extension == "font")
                fileType = "font";
            else if (extension == "layout")
                fileType = "layout";
            else if (extension == "imageset")
                fileType = "imageset";
            else if (extension == "anim")
                fileType = "animation";
            else if (extension == "scheme")
                fileType = "scheme";
            else if (extension == "looknfeel")
                fileType = "looknfeel";
            else if (extension == "py")
                fileType = "python_script";
            else if (extension == "lua")
                fileType = "lua_script";
            else if (extension == "xml")
                fileType = "xml";
            else if (extension == "txt")
                fileType = "text";
            else if (extension == "png" ||
                     extension == "jpg" ||
                     extension == "jpeg" ||
                     extension == "tga" ||
                     extension == "dds")
            {
                fileType = "bitmap";
            }

            setIcon(QIcon(":/icons/project_items/" + fileType + ".png"));

            break;
        }
        case Type::Folder:
        {
            // A hack to cause folders appear first when sorted
            // TODO: Override the sorting method and make this work more cleanly
            setText(" " + path);
            setIcon(QIcon(":/icons/project_items/folder.png"));
            break;
        }
        default:
        {
            assert(false); // TODO: error
            return;
        }
    }

    setData(path, Qt::UserRole + 2);
    _project->setModified();
}

QString CEGUIProjectItem::getPath() const
{
    assert(getType() != Type::Unknown);
    return data(Qt::UserRole + 2).toString();
}

// Returns path relative to the projects base directory
QString CEGUIProjectItem::getRelativePath() const
{
    assert(getType() == Type::File);
    return getPath();
}

QString CEGUIProjectItem::getAbsolutePath() const
{
    assert(getType() == Type::File);
    return _project->getAbsolutePathOf(getPath());
}

// Checks whether given absolute path is referenced by this Item
// or any of its descendants
bool CEGUIProjectItem::referencesFilePath(const QString& filePath) const
{
    auto type = getType();
    if (type == Type::File)
    {
        // Figuring out whether 2 paths lead to the same file is a tricky
        // business. We will do our best but this might not work in all cases!
        QFileInfo thisPath(QDir::cleanPath(getAbsolutePath()));
        QFileInfo otherPath(QDir::cleanPath(filePath));
        return thisPath == otherPath;
    }
    else if (type == Type::Folder)
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            auto itemPtr = static_cast<CEGUIProjectItem*>(child(i));
            if (itemPtr->referencesFilePath(filePath))
                return true;
        }
    }

    return false;
}
