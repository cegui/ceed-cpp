#ifndef QTNPROPERTYGLMVEC3_H
#define QTNPROPERTYGLMVEC3_H

#include "QtnProperty/GUI/PropertyQVector3D.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators and string conversion
#include <CEGUI/PropertyHelper.h>

class QtnPropertyGlmVec3Base : public QtnSinglePropertyBaseAs<QtnPropertyQVector3DBase, glm::vec3>
{
    Q_OBJECT

    QtnPropertyGlmVec3Base(const QtnPropertyGlmVec3Base &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnPropertyGlmVec3Base(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyQVector3DBase, glm::vec3>(parent)
    {
    }

protected:

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        baseValue.setX(actualValue.x);
        baseValue.setY(actualValue.y);
        baseValue.setZ(actualValue.z);
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        actualValue.x = baseValue.x();
        actualValue.y = baseValue.y();
        actualValue.z = baseValue.z();
        return true;
    }

    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<glm::vec3>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<glm::vec3>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyGlmVec3Base, glm::vec3)

class QtnPropertyGlmVec3Callback : public QtnSinglePropertyCallback<QtnPropertyGlmVec3Base>
{
    Q_OBJECT

    QtnPropertyGlmVec3Callback(const QtnPropertyGlmVec3Callback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyGlmVec3Callback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnPropertyGlmVec3Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertyGlmVec3Callback, QtnPropertyGlmVec3Base)
};

class QtnPropertyGlmVec3 : public QtnSinglePropertyValue<QtnPropertyGlmVec3Base>
{
    Q_OBJECT

    QtnPropertyGlmVec3(const QtnPropertyGlmVec3 &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyGlmVec3(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnPropertyGlmVec3Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyGlmVec3, QtnPropertyGlmVec3Base)
};

#endif // QTNPROPERTYGLMVEC3_H
