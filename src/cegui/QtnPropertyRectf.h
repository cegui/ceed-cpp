#ifndef QTNPROPERTYRECTF_H
#define QTNPROPERTYRECTF_H

#include "QtnProperty/Core/PropertyQRectF.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators and string conversion
#include <CEGUI/PropertyHelper.h>

class QtnPropertyRectfBase : public QtnSinglePropertyBaseAs<QtnPropertyQRectFBase, CEGUI::Rectf>
{
    Q_OBJECT

    QtnPropertyRectfBase(const QtnPropertyRectfBase &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnPropertyRectfBase(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyQRectFBase, CEGUI::Rectf>(parent)
    {
    }

protected:

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        baseValue.setLeft(actualValue.left());
        baseValue.setTop(actualValue.top());
        baseValue.setRight(actualValue.right());
        baseValue.setBottom(actualValue.bottom());
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        actualValue.d_min.x = static_cast<float>(baseValue.left());
        actualValue.d_min.y = static_cast<float>(baseValue.top());
        actualValue.d_max.x = static_cast<float>(baseValue.right());
        actualValue.d_max.y = static_cast<float>(baseValue.bottom());
        return true;
    }

    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::Rectf>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<CEGUI::Rectf>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyRectfBase, CEGUI::Rectf)

class QtnPropertyRectfCallback : public QtnSinglePropertyCallback<QtnPropertyRectfBase>
{
    Q_OBJECT

    QtnPropertyRectfCallback(const QtnPropertyRectfCallback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyRectfCallback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnPropertyRectfBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertyRectfCallback, QtnPropertyRectfBase)
};

class QtnPropertyRectf : public QtnSinglePropertyValue<QtnPropertyRectfBase>
{
    Q_OBJECT

    QtnPropertyRectf(const QtnPropertyRectf &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyRectf(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnPropertyRectfBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyRectf, QtnPropertyRectfBase)
};

#endif // QTNPROPERTYRECTF_H
