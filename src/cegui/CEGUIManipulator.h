#ifndef CEGUIMANIPULATOR_H
#define CEGUIMANIPULATOR_H

#include "src/ui/ResizableRectItem.h"
#include <CEGUI/UVector.h>
#include <CEGUI/Sizef.h>

// This is a rectangle that is synchronised with given CEGUI widget,
// it provides moving and resizing functionality

namespace CEGUI
{
    class Window;
}

class CEGUIManipulator : public ResizableRectItem
{
public:

    CEGUIManipulator(QGraphicsItem* parent = nullptr, CEGUI::Window* widget = nullptr);

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    virtual QSizeF getMinSize() const override;
    virtual QSizeF getMaxSize() const override;
    CEGUI::Sizef getBaseSize() const;

    virtual void notifyHandleSelected(ResizingHandle* handle) override;
    virtual void notifyResizeStarted(ResizingHandle* handle) override;
    virtual void notifyResizeProgress(QPointF newPos, QRectF newRect) override;
    virtual void notifyResizeFinished(QPointF newPos, QRectF newRect) override;
    virtual void notifyMoveStarted() override;
    virtual void notifyMoveProgress(QPointF newPos) override;
    virtual void notifyMoveFinished(QPointF newPos) override;

    virtual void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false);
    virtual void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true);

    // Returns whether the painting code should strive to prevent manipulator overlap (crossing outlines and possibly other things)
    virtual bool preventManipulatorOverlap() const { return false; }
    virtual bool useAbsoluteCoordsForMove() const { return false; }
    virtual bool useAbsoluteCoordsForResize() const { return false; }
    virtual bool useIntegersForAbsoluteMove() const { return false; }
    virtual bool useIntegersForAbsoluteResize() const { return false; }

    CEGUI::Window* getWidget() const { return _widget; }
    QString getWidgetName() const;
    QString getWidgetType() const;
    QString getWidgetPath() const;
    virtual CEGUIManipulator* createChildManipulator(CEGUI::Window* childWidget, bool recursive = true, bool skipAutoWidgets = false);
    void getChildManipulators(std::vector<CEGUIManipulator*>& outList, bool recursive);
    CEGUIManipulator* getManipulatorByPath(const QString& widgetPath) const;
    CEGUIManipulator* getManipulatorFromChildContainerByPath(const QString& widgetPath) const;
    void forEachChildWidget(std::function<void (CEGUI::Window*)> callback) const;

    void createChildManipulators(bool recursive, bool skipAutoWidgets, bool checkExisting = true);
    void moveToFront();
    void triggerPropertyManagerCallback(QStringList propertyNames);
    bool shouldBeSkipped() const;
    bool hasNonAutoWidgetDescendants() const;

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void impl_paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);

    CEGUI::Window* _widget = nullptr;

    // All below must be null at start!

    QPointF _lastResizeNewPos;
    QRectF _lastResizeNewRect;

    /*
            self.preResizePos = None
            self.preResizeSize = None
    */
    CEGUI::UVector2 _preMovePos;
    QPointF _lastMoveNewPos;
};

#endif // CEGUIMANIPULATOR_H
