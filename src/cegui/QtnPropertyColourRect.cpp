#include "QtnPropertyColourRect.h"
#include <src/cegui/CEGUIUtils.h>
#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include <qlineedit.h>

QtnPropertyColourRectBase::QtnPropertyColourRectBase(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyColourRectBase::createTopLeftProperty()
{
    return createFieldProperty(&CEGUI::ColourRect::d_top_left, &CEGUI::ColourRect::d_top_left,
                               QStringLiteral("top_left"), tr("Top-left"), tr("Top-left corner of %1"));
}

QtnProperty* QtnPropertyColourRectBase::createTopRightProperty()
{
    return createFieldProperty(&CEGUI::ColourRect::d_top_right, &CEGUI::ColourRect::d_top_right,
                               QStringLiteral("top_right"), tr("Top-right"), tr("Top-right corner of %1"));
}

QtnProperty* QtnPropertyColourRectBase::createBottomLeftProperty()
{
    return createFieldProperty(&CEGUI::ColourRect::d_bottom_left, &CEGUI::ColourRect::d_bottom_left,
                               QStringLiteral("bottom_left"), tr("Bottom-left"), tr("Bottom-left corner of %1"));
}

QtnProperty* QtnPropertyColourRectBase::createBottomRightProperty()
{
    return createFieldProperty(&CEGUI::ColourRect::d_bottom_right, &CEGUI::ColourRect::d_bottom_right,
                               QStringLiteral("bottom_right"), tr("Bottom-right"), tr("Bottom-right corner of %1"));
}

bool QtnPropertyColourRectBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::ColourRect>().fromString(CEGUIUtils::qStringToString(str)), reason);
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyColourRectBase::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::ColourRect>().toString(value()));
    return true;
}

QtnPropertyDelegateColourRect::QtnPropertyDelegateColourRect(QtnPropertyColourRectBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyColourRectBase>(owner)
{
    addSubProperty(owner.createTopLeftProperty());
    addSubProperty(owner.createTopRightProperty());
    addSubProperty(owner.createBottomLeftProperty());
    addSubProperty(owner.createBottomRightProperty());
}

void QtnPropertyDelegateColourRect::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyColourRectBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateColourRect, QtnPropertyColourRectBase>
                 , "ColourRect");
}

QWidget* QtnPropertyDelegateColourRect::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateColourRect::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
