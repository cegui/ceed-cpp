#ifndef CEGUIPROJECTITEM_H
#define CEGUIPROJECTITEM_H

#include "qstandarditemmodel.h"

// One item in the project. This is usually a file or a folder.

class CEGUIProject;

class CEGUIProjectItem : public QStandardItem
{
public:

    enum class Type
    {
        Unknown,

        // A file is an item that can't have any children, it is directly opened instead of
        // being expanded/collapsed like folders
        File,

        // Folder is a group of files. Project folders don't necessarily have to have
        // a counterpart on the HDD, they could be virtual.
        Folder
    };

    CEGUIProjectItem(CEGUIProject* project);

    // Qt docs say we have to overload type() and return something > QStandardItem.UserType
    virtual int type() const { return QStandardItem::UserType + 1; }
    virtual QStandardItem* clone() const;

    void setType(Type type);
    void setPath(const QString& path);
    QString getPath() const;
    QString getRelativePath() const;
    QString getAbsolutePath() const;
    bool referencesFilePath(const QString& filePath) const;

protected:

    CEGUIProject* _project = nullptr;
    Type _type = Type::Unknown;
    QString _path;
    QString _label;
    QString _icon;
};

#endif // CEGUIPROJECTITEM_H
