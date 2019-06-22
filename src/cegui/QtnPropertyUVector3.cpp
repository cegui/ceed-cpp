#include "QtnPropertyUVector3.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include <qlineedit.h>

QtnPropertyUVector3Base::QtnPropertyUVector3Base(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyUVector3Base::createXProperty()
{
    return createFieldProperty(&CEGUI::UVector3::d_x, &CEGUI::UVector3::d_x,
                               QStringLiteral("x"), tr("X"), tr("X of the %1"));
}

QtnProperty* QtnPropertyUVector3Base::createYProperty()
{
    return createFieldProperty(&CEGUI::UVector3::d_y, &CEGUI::UVector3::d_y,
                               QStringLiteral("y"), tr("Y"), tr("Y of the %1"));
}

QtnProperty* QtnPropertyUVector3Base::createZProperty()
{
    return createFieldProperty(&CEGUI::UVector3::d_z, &CEGUI::UVector3::d_z,
                               QStringLiteral("z"), tr("Z"), tr("Z of the %1"));
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

QtnPropertyDelegateUVector3::QtnPropertyDelegateUVector3(QtnPropertyUVector3Base& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUVector3Base>(owner)
{
    addSubProperty(owner.createXProperty());
    addSubProperty(owner.createYProperty());
    addSubProperty(owner.createZProperty());
}

void QtnPropertyDelegateUVector3::Register(QtnPropertyDelegateFactory &factory)
{
    factory.registerDelegateDefault(&QtnPropertyUVector3Base::staticMetaObject,
        &qtnCreateDelegate<QtnPropertyDelegateUVector3, QtnPropertyUVector3Base>,
        "UVector3");
}

QWidget* QtnPropertyDelegateUVector3::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUVector3::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
