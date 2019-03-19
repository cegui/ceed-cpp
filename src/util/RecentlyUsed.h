#ifndef RECENTLYUSED_H
#define RECENTLYUSED_H

#include "qstring.h"
#include "qmenu.h"

// Implements reusable functionality for "recently used" lists/menus.
// Can be used to store pointers to Items like files and images
// for later reuse within the application.

// Stefan Stammberger is the author of the original python implementation

// This class can be used to store pointers to Items like files and images
// for later reuse within the application.
class RecentlyUsed : public QObject
{
    Q_OBJECT

public:

    RecentlyUsed(const QString& sectionID, QObject* parent = nullptr);

    virtual void addRecentlyUsed(const QString& name);
    virtual bool removeRecentlyUsed(const QString& name);
    void getRecentlyUsed(QStringList& outList) const;

public slots:

    virtual void clearRecentlyUsed();

signals:

    void triggered(const QString& itemName);

protected:

    QString _sectionID;
    int _maxItems = 10;
};

// This class can be used to manage a Qt Menu entry to items
class RecentlyUsedMenuEntry : public RecentlyUsed
{
    Q_OBJECT

public:

    RecentlyUsedMenuEntry(const QString& sectionID, QObject* parent = nullptr);

    void setParentMenu(QMenu* menu);
    virtual void addRecentlyUsed(const QString& name) override;
    virtual bool removeRecentlyUsed(const QString& name) override;

public slots:

    virtual void clearRecentlyUsed() override;

protected slots:

    void recentItemActionTriggered();

protected:

    void updateMenu();

    QMenu* _menu = nullptr;
};

#endif // RECENTLYUSED_H
