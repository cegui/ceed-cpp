#include "QtnPropertyUBox.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>

QtnPropertyUBoxBase::QtnPropertyUBoxBase(QObject* parent)
    : QtnSinglePropertyBase<CEGUI::UBox>(parent)
{
}

bool QtnPropertyUBoxBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        setValue(CEGUI::PropertyHelper<CEGUI::UBox>().fromString(CEGUIUtils::qStringToString(str)));
        return true;
    }
    catch (...)
    {
    }

    return false;
}

bool QtnPropertyUBoxBase::toStrImpl(QString& str) const
{
    str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::UBox>().toString(value()));
    return true;
}

QtnProperty* qtnCreateTopProperty(QObject* parent, QtnPropertyUBoxBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Top"));
    subproperty->setDescription(QObject::tr("Top of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_top; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UBox value = mainProperty->value();
        value.d_top = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateBottomProperty(QObject* parent, QtnPropertyUBoxBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Bottom"));
    subproperty->setDescription(QObject::tr("Bottom of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_bottom; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UBox value = mainProperty->value();
        value.d_bottom = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateLeftProperty(QObject* parent, QtnPropertyUBoxBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Left"));
    subproperty->setDescription(QObject::tr("Left of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_left; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UBox value = mainProperty->value();
        value.d_left = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QtnProperty* qtnCreateRightProperty(QObject* parent, QtnPropertyUBoxBase* mainProperty)
{
    QtnPropertyUDimCallback* subproperty = new QtnPropertyUDimCallback(parent);
    subproperty->setName(QObject::tr("Right"));
    subproperty->setDescription(QObject::tr("Right of %1.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->CEGUI::UDim { return mainProperty->value().d_right; });
    subproperty->setCallbackValueSet([mainProperty](CEGUI::UDim newValue) {
        CEGUI::UBox value = mainProperty->value();
        value.d_right = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

void qtnRegisterUBoxDelegates(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUBoxBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUBox, QtnPropertyUBoxBase>
                 , "UBox");
}

QtnPropertyDelegateUBox::QtnPropertyDelegateUBox(QtnPropertyUBoxBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUBoxBase>(owner)
{
    addSubProperty(qtnCreateLeftProperty(nullptr, &owner));
    addSubProperty(qtnCreateTopProperty(nullptr, &owner));
    addSubProperty(qtnCreateRightProperty(nullptr, &owner));
    addSubProperty(qtnCreateBottomProperty(nullptr, &owner));
}

QWidget* QtnPropertyDelegateUBox::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUBox::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
