#include "QtnPropertyUDim.h"
#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include "qlineedit.h"

QtnPropertyUDimBase::QtnPropertyUDimBase(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyUDimBase::createScaleProperty()
{
    auto subproperty = createFieldProperty(&CEGUI::UDim::d_scale, &CEGUI::UDim::d_scale,
                               QStringLiteral("scale"), tr("Scale"), tr("Relative part of %1"));
    subproperty->setStepValue(0.05f);
    return subproperty;
}

QtnProperty* QtnPropertyUDimBase::createOffsetProperty()
{
    auto subproperty = createFieldProperty(&CEGUI::UDim::d_offset, &CEGUI::UDim::d_offset,
                               QStringLiteral("offset"), tr("Offset"), tr("Absolute part of %1"));
    subproperty->setStepValue(1.f);
    return subproperty;
}

bool QtnPropertyUDimBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::UDim>().fromString(CEGUIUtils::qStringToString(str)), reason);
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

QtnPropertyDelegateUDim::QtnPropertyDelegateUDim(QtnPropertyUDimBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUDimBase>(owner)
{
    addSubProperty(owner.createScaleProperty());
    addSubProperty(owner.createOffsetProperty());
}

void QtnPropertyDelegateUDim::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUDimBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUDim, QtnPropertyUDimBase>
                 , "UDim");
}

QWidget* QtnPropertyDelegateUDim::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUDim::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
