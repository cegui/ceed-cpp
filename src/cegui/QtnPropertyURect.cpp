#include "QtnPropertyURect.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyURectBase::QtnPropertyURectBase(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::URect>(parent)
{
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

QtnProperty* qtnCreateMinXProperty(QObject* parent, QtnPropertyURectBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("MinX"));
    subproperty->setDescription(QObject::tr("Minimum X of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_min.d_x; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::URect value = mainProperty->value();
        value.d_min.d_x = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateMinYProperty(QObject* parent, QtnPropertyURectBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("MinY"));
    subproperty->setDescription(QObject::tr("Minimum Y of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_min.d_y; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::URect value = mainProperty->value();
        value.d_min.d_y = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateMaxXProperty(QObject* parent, QtnPropertyURectBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("MaxX"));
    subproperty->setDescription(QObject::tr("Maximum X of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_max.d_x; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::URect value = mainProperty->value();
        value.d_max.d_x = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateMaxYProperty(QObject* parent, QtnPropertyURectBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("MaxY"));
    subproperty->setDescription(QObject::tr("Maximum Y of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_max.d_y; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::URect value = mainProperty->value();
        value.d_max.d_y = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterURectDelegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyURectBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateURect, QtnPropertyURectBase>
                 , "URect");
}

QtnPropertyDelegateURect::QtnPropertyDelegateURect(QtnPropertyURectBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyURectBase>(owner)
{
    addSubProperty(qtnCreateMinXProperty(nullptr, &owner));
    addSubProperty(qtnCreateMinYProperty(nullptr, &owner));
    addSubProperty(qtnCreateMaxXProperty(nullptr, &owner));
    addSubProperty(qtnCreateMaxYProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateURect::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateURect::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
