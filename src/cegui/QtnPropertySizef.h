#ifndef QTNPROPERTYSIZEF_H
#define QTNPROPERTYSIZEF_H

#include "QtnProperty/Core/PropertyQSizeF.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators and string conversion
#include <CEGUI/PropertyHelper.h>

class QtnPropertySizefBase : public QtnSinglePropertyBaseAs<QtnPropertyQSizeFBase, CEGUI::Sizef>
{
    Q_OBJECT

    QtnPropertySizefBase(const QtnPropertySizefBase &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnPropertySizefBase(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyQSizeFBase, CEGUI::Sizef>(parent)
    {
    }

protected:

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        baseValue.setWidth(actualValue.d_width);
        baseValue.setHeight(actualValue.d_height);
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        actualValue.d_width = baseValue.width();
        actualValue.d_height = baseValue.height();
        return true;
    }

    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::Sizef>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<CEGUI::Sizef>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertySizefBase, CEGUI::Sizef)

class QtnPropertySizefCallback : public QtnSinglePropertyCallback<QtnPropertySizefBase>
{
    Q_OBJECT

    QtnPropertySizefCallback(const QtnPropertySizefCallback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertySizefCallback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnPropertySizefBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertySizefCallback, QtnPropertySizefBase)
};

class QtnPropertySizef : public QtnSinglePropertyValue<QtnPropertySizefBase>
{
    Q_OBJECT

    QtnPropertySizef(const QtnPropertySizef &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertySizef(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnPropertySizefBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertySizef, QtnPropertySizefBase)
};

#endif // QTNPROPERTYSIZEF_H
