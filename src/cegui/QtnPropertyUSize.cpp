#include "QtnPropertyUSize.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyUSizeBase::QtnPropertyUSizeBase(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::USize>(parent)
{
}

bool QtnPropertyUSizeBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::USize>().fromString(CEGUIUtils::qStringToString(str)), reason);
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyUSizeBase::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::USize>().toString(value()));
    return true;
}

QtnProperty* qtnCreateWidthProperty(QObject* parent, QtnPropertyUSizeBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Width"));
    subproperty->setDescription(QObject::tr("Width component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_width; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::USize value = mainProperty->value();
        value.d_width = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateHeightProperty(QObject* parent, QtnPropertyUSizeBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Height"));
    subproperty->setDescription(QObject::tr("Height component of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_height; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::USize value = mainProperty->value();
        value.d_height = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterUSizeDelegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUSizeBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUSize, QtnPropertyUSizeBase>
                 , "USize");
}

QtnPropertyDelegateUSize::QtnPropertyDelegateUSize(QtnPropertyUSizeBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUSizeBase>(owner)
{
    addSubProperty(qtnCreateWidthProperty(nullptr, &owner));
    addSubProperty(qtnCreateHeightProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateUSize::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUSize::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
