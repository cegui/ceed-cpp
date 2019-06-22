#include "QtnPropertyUVector2.h"
#include "QtnPropertyUDim.h" // UDim or Float!
#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include "qlineedit.h"

QtnPropertyUVector2Base::QtnPropertyUVector2Base(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyUVector2Base::createXProperty()
{
    return createFieldProperty(&CEGUI::UVector2::d_x, &CEGUI::UVector2::d_x,
                               QStringLiteral("x"), tr("X"), tr("X of the %1"));
}

QtnProperty* QtnPropertyUVector2Base::createYProperty()
{
    return createFieldProperty(&CEGUI::UVector2::d_y, &CEGUI::UVector2::d_y,
                               QStringLiteral("y"), tr("Y"), tr("Y of the %1"));
}

bool QtnPropertyUVector2Base::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::UVector2>().fromString(CEGUIUtils::qStringToString(str)), reason);
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

QtnPropertyDelegateUVector2::QtnPropertyDelegateUVector2(QtnPropertyUVector2Base& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUVector2Base>(owner)
{
    addSubProperty(owner.createXProperty());
    addSubProperty(owner.createYProperty());
}

void QtnPropertyDelegateUVector2::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUVector2Base::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUVector2, QtnPropertyUVector2Base>
                 , "UVector2");
}

QWidget* QtnPropertyDelegateUVector2::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUVector2::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
