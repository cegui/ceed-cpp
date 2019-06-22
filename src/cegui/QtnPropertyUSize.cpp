#include "QtnPropertyUSize.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include "qlineedit.h"

QtnPropertyUSizeBase::QtnPropertyUSizeBase(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyUSizeBase::createWidthProperty()
{
    return createFieldProperty(&CEGUI::USize::d_width, &CEGUI::USize::d_width,
                               QStringLiteral("width"), tr("Width"), tr("Width of the %1"));
}

QtnProperty* QtnPropertyUSizeBase::createHeightProperty()
{
    return createFieldProperty(&CEGUI::USize::d_height, &CEGUI::USize::d_height,
                               QStringLiteral("height"), tr("Height"), tr("Height of the %1"));
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

QtnPropertyDelegateUSize::QtnPropertyDelegateUSize(QtnPropertyUSizeBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUSizeBase>(owner)
{
    addSubProperty(owner.createWidthProperty());
    addSubProperty(owner.createHeightProperty());
}

void QtnPropertyDelegateUSize::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUSizeBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUSize, QtnPropertyUSizeBase>
                 , "USize");
}

QWidget* QtnPropertyDelegateUSize::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUSize::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
