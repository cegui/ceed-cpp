#include "QtnPropertyUVector2.h"
#include "src/cegui/CEGUIUtils.h"
#include "3rdParty/QtnProperty/Core/Core/PropertyFloat.h"
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
    subproperty->setName(QObject::tr("Rel X"));
    subproperty->setDescription(QObject::tr("Relative part of %1's X.").arg(mainProperty->name()));
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
    subproperty->setName(QObject::tr("Abs X"));
    subproperty->setDescription(QObject::tr("Absolute part of %1's X.").arg(mainProperty->name()));
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
    subproperty->setName(QObject::tr("Rel Y"));
    subproperty->setDescription(QObject::tr("Relative part of %1's Y.").arg(mainProperty->name()));
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
    subproperty->setName(QObject::tr("Abs Y"));
    subproperty->setDescription(QObject::tr("Absolute part of %1's Y.").arg(mainProperty->name()));
    subproperty->setCallbackValueGet([mainProperty]()->float { return mainProperty->value().d_y.d_offset; });
    subproperty->setCallbackValueSet([mainProperty](float newValue) {
        CEGUI::UVector2 value = mainProperty->value();
        value.d_y.d_offset = newValue;
        mainProperty->setValue(value);
    });
    QtnPropertyBase::connectMasterSignals(*mainProperty, *subproperty);

    return subproperty;
}

QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector2& value)
{
    stream << value.d_x.d_scale;
    stream << value.d_x.d_offset;
    stream << value.d_y.d_scale;
    stream << value.d_y.d_offset;
    return stream;
}

QDataStream& operator >>(QDataStream& stream, CEGUI::UVector2& value)
{
    stream >> value.d_x.d_scale;
    stream >> value.d_x.d_offset;
    stream >> value.d_y.d_scale;
    stream >> value.d_y.d_offset;
    return stream;
}
