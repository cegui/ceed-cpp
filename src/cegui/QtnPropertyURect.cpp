#include "QtnPropertyURect.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include "qlineedit.h"

QtnPropertyURectBase::QtnPropertyURectBase(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyURectBase::createMinXProperty()
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(nullptr);
    subproperty->setName(QObject::tr("MinX"));
    subproperty->setDescription(QObject::tr("Minimum X of %1").arg(name()));
    subproperty->setCallbackValueGet([this]()->CEGUI::UDim { return value().d_min.d_x; });
    subproperty->setCallbackValueSet([this](CEGUI::UDim newValue) {
        CEGUI::URect ownerValue = value();
        ownerValue.d_min.d_x = newValue;
        setValue(ownerValue);
    });
    return subproperty;
}

QtnProperty* QtnPropertyURectBase::createMinYProperty()
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(nullptr);
    subproperty->setName(QObject::tr("MinY"));
    subproperty->setDescription(QObject::tr("Minimum Y of %1").arg(name()));
    subproperty->setCallbackValueGet([this]()->CEGUI::UDim { return value().d_min.d_y; });
    subproperty->setCallbackValueSet([this](CEGUI::UDim newValue) {
        CEGUI::URect ownerValue = value();
        ownerValue.d_min.d_y = newValue;
        setValue(ownerValue);
    });
    return subproperty;
}

QtnProperty* QtnPropertyURectBase::createMaxXProperty()
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(nullptr);
    subproperty->setName(QObject::tr("MaxX"));
    subproperty->setDescription(QObject::tr("Maximum X of %1").arg(name()));
    subproperty->setCallbackValueGet([this]()->CEGUI::UDim { return value().d_max.d_x; });
    subproperty->setCallbackValueSet([this](CEGUI::UDim newValue) {
        CEGUI::URect ownerValue = value();
        ownerValue.d_max.d_x = newValue;
        setValue(ownerValue);
    });
    return subproperty;
}

QtnProperty* QtnPropertyURectBase::createMaxYProperty()
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(nullptr);
    subproperty->setName(QObject::tr("MaxY"));
    subproperty->setDescription(QObject::tr("Maximum Y of %1").arg(name()));
    subproperty->setCallbackValueGet([this]()->CEGUI::UDim { return value().d_max.d_y; });
    subproperty->setCallbackValueSet([this](CEGUI::UDim newValue) {
        CEGUI::URect ownerValue = value();
        ownerValue.d_max.d_y = newValue;
        setValue(ownerValue);
    });
    return subproperty;
}

bool QtnPropertyURectBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::URect>().fromString(CEGUIUtils::qStringToString(str)), reason);
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyURectBase::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::URect>().toString(value()));
    return true;
}

QtnPropertyDelegateURect::QtnPropertyDelegateURect(QtnPropertyURectBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyURectBase>(owner)
{
    addSubProperty(owner.createMinXProperty());
    addSubProperty(owner.createMinYProperty());
    addSubProperty(owner.createMaxXProperty());
    addSubProperty(owner.createMaxYProperty());
}

void QtnPropertyDelegateURect::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyURectBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateURect, QtnPropertyURectBase>
                 , "URect");
}

QWidget* QtnPropertyDelegateURect::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateURect::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
