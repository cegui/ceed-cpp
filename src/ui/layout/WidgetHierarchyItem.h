#ifndef WIDGETHIERARCHYITEM_H
#define WIDGETHIERARCHYITEM_H

#include "qstandarditemmodel.h"

class LayoutManipulator;

class WidgetHierarchyItem : public QStandardItem
{
public:

    WidgetHierarchyItem(LayoutManipulator* manipulator);

    virtual QStandardItem* clone() const override;
    virtual void setData(const QVariant& value, int role = Qt::UserRole + 1) override;

    void refreshPathData(bool recursive = true);
    void refreshOrderingData(bool resort = true, bool recursive = true);
    void setLocked(bool locked, bool recursive = false);
    LayoutManipulator* getManipulator() const { return _manipulator; }

protected:

    LayoutManipulator* _manipulator = nullptr;
};

#endif // WIDGETHIERARCHYITEM_H
