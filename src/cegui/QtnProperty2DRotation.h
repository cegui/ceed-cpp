#ifndef QTNPROPERTY2DRotation_H
#define QTNPROPERTY2DRotation_H

#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators and string conversion
#include <CEGUI/PropertyHelper.h>

// Simplified 2D rotation edited as a float angle in degrees and saved as a glm::quat
class QtnProperty2DRotationBase : public QtnSinglePropertyBaseAs<QtnPropertyFloatBase, glm::quat>
{
    Q_OBJECT

    QtnProperty2DRotationBase(const QtnProperty2DRotationBase &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnProperty2DRotationBase(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyFloatBase, glm::quat>(parent)
    {
    }

protected:

    static float normalizeAngle(float angle)
    {
        angle = std::fmodf(angle, 360.f);
        if (angle < 0.f) angle += 360.f;
        return angle;
    }

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        // TODO: support extracting a 2D angle from quaternion rotated not along Z
#ifdef GLM_FORCE_RADIANS
        baseValue = normalizeAngle(glm::degrees(glm::angle(actualValue)));
#else
        baseValue = normalizeAngle(glm::angle(actualValue));
#endif
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        // Z axis goes through the screen, 2D objects are rotated around it
#ifdef GLM_FORCE_RADIANS
        actualValue = glm::angleAxis(glm::radians(normalizeAngle(baseValue)), 0.f, 0.f, 1.f);
#else
        actualValue = glm::angleAxis(normalizeAngle(baseValue), 0.f, 0.f, 1.f);
#endif
        return true;
    }

    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<glm::quat>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<glm::quat>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnProperty2DRotationBase, glm::quat)

class QtnProperty2DRotationCallback : public QtnSinglePropertyCallback<QtnProperty2DRotationBase>
{
    Q_OBJECT

    QtnProperty2DRotationCallback(const QtnProperty2DRotationCallback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnProperty2DRotationCallback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnProperty2DRotationBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnProperty2DRotationCallback, QtnProperty2DRotationBase)
};

class QtnProperty2DRotation : public QtnSinglePropertyValue<QtnProperty2DRotationBase>
{
    Q_OBJECT

    QtnProperty2DRotation(const QtnProperty2DRotation &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnProperty2DRotation(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnProperty2DRotationBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnProperty2DRotation, QtnProperty2DRotationBase)
};

#endif // QTNPROPERTY2DRotation_H
