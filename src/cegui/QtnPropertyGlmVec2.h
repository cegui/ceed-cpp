#ifndef QTNPROPERTYGLMVEC2_H
#define QTNPROPERTYGLMVEC2_H

#include "QtnProperty/Core/PropertyQPointF.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators and string conversion
#include <CEGUI/PropertyHelper.h>

class QtnPropertyGlmVec2Base : public QtnSinglePropertyBaseAs<QtnPropertyQPointFBase, glm::vec2>
{
    Q_OBJECT

    QtnPropertyGlmVec2Base(const QtnPropertyGlmVec2Base &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnPropertyGlmVec2Base(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyQPointFBase, glm::vec2>(parent)
    {
    }

protected:

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        baseValue.setX(actualValue.x);
        baseValue.setY(actualValue.y);
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        actualValue.x = baseValue.x();
        actualValue.y = baseValue.y();
        return true;
    }

    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<glm::vec2>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<glm::vec2>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyGlmVec2Base, glm::vec2)

class QtnPropertyGlmVec2Callback : public QtnSinglePropertyCallback<QtnPropertyGlmVec2Base>
{
    Q_OBJECT

    QtnPropertyGlmVec2Callback(const QtnPropertyGlmVec2Callback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyGlmVec2Callback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnPropertyGlmVec2Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertyGlmVec2Callback, QtnPropertyGlmVec2Base)
};

class QtnPropertyGlmVec2 : public QtnSinglePropertyValue<QtnPropertyGlmVec2Base>
{
    Q_OBJECT

    QtnPropertyGlmVec2(const QtnPropertyGlmVec2 &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyGlmVec2(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnPropertyGlmVec2Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyGlmVec2, QtnPropertyGlmVec2Base)
};

#endif // QTNPROPERTYGLMVEC2_H
