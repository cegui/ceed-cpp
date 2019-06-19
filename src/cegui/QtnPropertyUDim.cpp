#include "QtnPropertyUDim.h"
#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyUDimBase::QtnPropertyUDimBase(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::UDim>(parent)
{
}

bool QtnPropertyUDimBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        setValue(CEGUI::PropertyHelper<CEGUI::UDim>().fromString(CEGUIUtils::qStringToString(str)));
        return true;
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyUDimBase::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::UDim>().toString(value()));
    return true;
}

QtnProperty* qtnCreateScaleProperty(QObject* parent, QtnPropertyUDimBase* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("Scale"));
    subproperty->setDescription(QObject::tr("Relative part of %1.").arg(mainProperty->name()));
    subproperty->setStepValue(0.05f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_scale; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UDim value = mainProperty->value();
        value.d_scale = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateOffsetProperty(QObject* parent, QtnPropertyUDimBase* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("Offset"));
    subproperty->setDescription(QObject::tr("Absolute part of %1.").arg(mainProperty->name()));
    subproperty->setStepValue(1.0f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_offset; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UDim value = mainProperty->value();
        value.d_offset = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterUDimDelegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUDimBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUDim, QtnPropertyUDimBase>
                 , "UDim");
}

QtnPropertyDelegateUDim::QtnPropertyDelegateUDim(QtnPropertyUDimBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUDimBase>(owner)
{
    addSubProperty(qtnCreateScaleProperty(nullptr, &owner));
    addSubProperty(qtnCreateOffsetProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateUDim::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUDim::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
