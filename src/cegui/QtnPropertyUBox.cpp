#include "QtnPropertyUBox.h"
#include "QtnPropertyUDim.h"
#include "QtnProperty/Delegates/PropertyDelegateFactory.h"
#include <CEGUI/PropertyHelper.h>
#include "qlineedit.h"

QtnPropertyUBoxBase::QtnPropertyUBoxBase(QObject* parent)
    : ParentClass(parent)
{
}

QtnProperty* QtnPropertyUBoxBase::createTopProperty()
{
    return createFieldProperty(&CEGUI::UBox::d_top, &CEGUI::UBox::d_top,
                               QStringLiteral("top"), tr("Top"), tr("Top of the %1"));
}

QtnProperty* QtnPropertyUBoxBase::createBottomProperty()
{
    return createFieldProperty(&CEGUI::UBox::d_bottom, &CEGUI::UBox::d_bottom,
                               QStringLiteral("bottom"), tr("Bottom"), tr("Bottom of the %1"));
}

QtnProperty* QtnPropertyUBoxBase::createLeftProperty()
{
    return createFieldProperty(&CEGUI::UBox::d_left, &CEGUI::UBox::d_left,
                               QStringLiteral("left"), tr("Left"), tr("Left of the %1"));
}

QtnProperty* QtnPropertyUBoxBase::createRightProperty()
{
    return createFieldProperty(&CEGUI::UBox::d_right, &CEGUI::UBox::d_right,
                               QStringLiteral("right"), tr("Right"), tr("Right of the %1"));
}

bool QtnPropertyUBoxBase::fromStrImpl(const QString& str, QtnPropertyChangeReason reason)
{
    try
    {
        return setValue(CEGUI::PropertyHelper<CEGUI::UBox>().fromString(CEGUIUtils::qStringToString(str)), reason);
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

QtnPropertyDelegateUBox::QtnPropertyDelegateUBox(QtnPropertyUBoxBase& owner)
    : QtnPropertyDelegateTypedEx<QtnPropertyUBoxBase>(owner)
{
    addSubProperty(owner.createLeftProperty());
    addSubProperty(owner.createTopProperty());
    addSubProperty(owner.createRightProperty());
    addSubProperty(owner.createBottomProperty());
}

void QtnPropertyDelegateUBox::Register(QtnPropertyDelegateFactory& factory)
{
    factory.registerDelegateDefault(&QtnPropertyUBoxBase::staticMetaObject
                 , &qtnCreateDelegate<QtnPropertyDelegateUBox, QtnPropertyUBoxBase>
                 , "UBox");
}

QWidget* QtnPropertyDelegateUBox::createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo)
{
    return createValueEditorLineEdit(parent, rect, true, inplaceInfo);
}

bool QtnPropertyDelegateUBox::propertyValueToStrImpl(QString& strValue) const
{
    return owner().toStr(strValue);
}
