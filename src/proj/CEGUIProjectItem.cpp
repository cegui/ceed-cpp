#include "src/proj/CEGUIProjectItem.h"
#include "src/proj/CEGUIProject.h"
#include "qdir.h"

CEGUIProjectItem::CEGUIProjectItem(CEGUIProject* project)
    : _project(project)
{
    project->setModified();
}

QStandardItem* CEGUIProjectItem::clone() const
{
    CEGUIProjectItem* ret = new CEGUIProjectItem(_project);
    ret->_type = _type;

    if (_type == Type::File || _type == Type::Folder)
        ret->_path = _path;

    return ret;
}

void CEGUIProjectItem::setType(CEGUIProjectItem::Type type)
{
    setData(static_cast<int>(type), Qt::UserRole + 1);

    // We can drag files but we can't drop anything to them
    // We can drag folders and drop other items to them
    setDragEnabled(_type == Type::File || _type == Type::Folder);
    setDropEnabled(_type == Type::Folder);
}

void CEGUIProjectItem::setPath(const QString& path)
{
    switch (_type)
    {
        case Type::File:
        {
            QFileInfo pathInfo(path);
            _label = pathInfo.fileName();

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

            _icon = ":/icons/project_items/" + fileType + ".png";

            break;
        }
        case Type::Folder:
        {
            // A hack to cause folders appear first when sorted
            // TODO: Override the sorting method and make this work more cleanly
            _label = " " + path;
            _icon = ":/icons/project_items/folder.png";
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
    assert(_type == Type::Unknown);
    return data(Qt::UserRole + 2).toString();
}

// Returns path relative to the projects base directory
QString CEGUIProjectItem::getRelativePath() const
{
    assert(_type == Type::File);
    return _path;
}

QString CEGUIProjectItem::getAbsolutePath() const
{
    assert(_type == Type::File);
    return _project->getAbsolutePathOf(_path);
}

// Checks whether given absolute path is referenced by this Item
// or any of its descendants
bool CEGUIProjectItem::referencesFilePath(const QString& filePath) const
{
    if (_type == Type::File)
    {
        // Figuring out whether 2 paths lead to the same file is a tricky
        // business. We will do our best but this might not work in all cases!
        QFileInfo thisPath(QDir::cleanPath(getAbsolutePath()));
        QFileInfo otherPath(QDir::cleanPath(filePath));
        return thisPath == otherPath;
    }
    else if (_type == Type::Folder)
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
