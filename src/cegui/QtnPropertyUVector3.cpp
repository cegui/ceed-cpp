#include "QtnPropertyUVector3.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyUVector3Base::QtnPropertyUVector3Base(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::UVector3>(parent)
{
}

bool QtnPropertyUVector3Base::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::UVector3>().fromString(CEGUIUtils::qStringToString(str)), reason);
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyUVector3Base::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::UVector3>().toString(value()));
    return true;
}

QtnProperty* QtnPropertyUVector3Base::createXProperty()
{
    return createFieldProperty(QObject::tr("X"),
        name(),
        QObject::tr("X component of %1."),
        &QRectF::height,
        &QRectF::setHeight);
}

QtnProperty* qtnCreateXProperty(QObject* parent, QtnPropertyUVector3Base* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("X"));
    subproperty->setDescription(QObject::tr("X component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_x; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UVector3 value = mainProperty->value();
        value.d_x = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateYProperty(QObject* parent, QtnPropertyUVector3Base* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Y"));
    subproperty->setDescription(QObject::tr("Y component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_y; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UVector3 value = mainProperty->value();
        value.d_y = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateZProperty(QObject* parent, QtnPropertyUVector3Base* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Z"));
    subproperty->setDescription(QObject::tr("Z component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_z; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UVector3 value = mainProperty->value();
        value.d_z = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterUVector3Delegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUVector3Base::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUVector3, QtnPropertyUVector3Base>
                 , "UVector3");
}

QtnPropertyDelegateUVector3::QtnPropertyDelegateUVector3(QtnPropertyUVector3Base& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUVector3Base>(owner)
{
    addSubProperty(qtnCreateXProperty(nullptr, &owner));
    addSubProperty(qtnCreateYProperty(nullptr, &owner));
    addSubProperty(qtnCreateZProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateUVector3::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUVector3::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
