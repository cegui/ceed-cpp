#include "QtnPropertyUVector2.h"
#include "QtnPropertyUDim.h" // UDim or Float!
#include "3rdParty/QtnProperty/Core/Core/PropertyFloat.h"
#include "3rdParty/QtnProperty/PropertyWidget/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyUVector2Base::QtnPropertyUVector2Base(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::UVector2>(parent)
{
}

bool QtnPropertyUVector2Base::fromStrImpl(const QString& str)
{
    try
    {
        setValue(CEGUI::PropertyHelper<CEGUI::UVector2>().fromString(CEGUIUtils::qStringToString(str)));
        return true;
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyUVector2Base::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::UVector2>().toString(value()));
    return true;
}

QtnProperty* qtnCreateXScaleProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("XScale"));
    subproperty->setDisplayName(QObject::tr("X rel."));
    subproperty->setDescription(QObject::tr("Relative part of %1's X.").arg(mainProperty->name()));
    subproperty->setStepValue(0.05f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_x.d_scale; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_x.d_scale = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateXOffsetProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("XOffset"));
    subproperty->setDisplayName(QObject::tr("X abs."));
    subproperty->setDescription(QObject::tr("Absolute part of %1's X.").arg(mainProperty->name()));
    subproperty->setStepValue(1.0f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_x.d_offset; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_x.d_offset = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateYScaleProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("YScale"));
    subproperty->setDisplayName(QObject::tr("Y rel."));
    subproperty->setDescription(QObject::tr("Relative part of %1's Y.").arg(mainProperty->name()));
    subproperty->setStepValue(0.05f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_y.d_scale; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_y.d_scale = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateYOffsetProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyFloatCallback* subproperty = new QtnPropertyFloatCallback(parent);
    subproperty->setName(QObject::tr("YOffset"));
    subproperty->setDisplayName(QObject::tr("Y abs."));
    subproperty->setDescription(QObject::tr("Absolute part of %1's Y.").arg(mainProperty->name()));
    subproperty->setStepValue(1.0f);
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_y.d_offset; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_y.d_offset = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

//!!!DBG TMP!
QtnProperty* qtnCreateXProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("X"));
    subproperty->setDescription(QObject::tr("X component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_x; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_x = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}
QtnProperty* qtnCreateYProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Y"));
    subproperty->setDescription(QObject::tr("Y component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_y; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_y = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterUVector2Delegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUVector2Base::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUVector2, QtnPropertyUVector2Base>
                 , "UVector2");
}

QtnPropertyDelegateUVector2::QtnPropertyDelegateUVector2(QtnPropertyUVector2Base& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUVector2Base>(owner)
{
    addSubProperty(qtnCreateXProperty(nullptr, &owner));
    addSubProperty(qtnCreateYProperty(nullptr, &owner));
    //addSubProperty(qtnCreateXScaleProperty(nullptr, &owner));
    //addSubProperty(qtnCreateXOffsetProperty(nullptr, &owner));
    //addSubProperty(qtnCreateYScaleProperty(nullptr, &owner));
    //addSubProperty(qtnCreateYOffsetProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateUVector2::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUVector2::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
