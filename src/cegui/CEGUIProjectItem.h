#ifndef CEGUIPROJECTITEM_H
#define CEGUIPROJECTITEM_H

#include "qstandarditemmodel.h"
#include "qdom.h"

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

    static Type getItemType(const QModelIndex& index);
    static bool isFile(const QModelIndex& index);
    static bool isFolder(const QModelIndex& index);

    CEGUIProjectItem(CEGUIProject* project);
    virtual ~CEGUIProjectItem() override;

    // Qt docs say we have to overload type() and return something > QStandardItem.UserType
    virtual int type() const { return QStandardItem::UserType + 1; }
    virtual QStandardItem* clone() const;

    void loadFromElement(const QDomElement& xml);
    bool saveToElement(QDomElement& xml);

    void setType(Type type);
    Type getType() const;
    void setPath(const QString& path);
    QString getPath() const;
    QString getRelativePath() const;
    QString getAbsolutePath() const;
    bool referencesFilePath(const QString& filePath) const;

protected:

    CEGUIProject* _project = nullptr;
};

#endif // CEGUIPROJECTITEM_H
