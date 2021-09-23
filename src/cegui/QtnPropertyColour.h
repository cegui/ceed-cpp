#ifndef QTNPROPERTYCOLOUR_H
#define QTNPROPERTYCOLOUR_H

#include "QtnProperty/GUI/PropertyQColor.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include <src/cegui/CEGUIUtils.h> // for stream operators
#include <CEGUI/Colour.h>
#include <CEGUI/PropertyHelper.h>

class QtnPropertyColourBase : public QtnSinglePropertyBaseAs<QtnPropertyQColorBase, CEGUI::Colour>
{
    Q_OBJECT

    QtnPropertyColourBase(const QtnPropertyColourBase &other) Q_DECL_EQ_DELETE;

public:

    explicit QtnPropertyColourBase(QObject* parent = nullptr)
        : QtnSinglePropertyBaseAs<QtnPropertyQColorBase, CEGUI::Colour>(parent)
    {
    }

protected:

    bool fromActualValue(ValueType actualValue, BaseValueTypeStore& baseValue) const override
    {
        baseValue.setRedF(actualValue.getRed());
        baseValue.setGreenF(actualValue.getGreen());
        baseValue.setBlueF(actualValue.getBlue());
        baseValue.setAlphaF(actualValue.getAlpha());
        return true;
    }

    bool toActualValue(ValueTypeStore& actualValue, BaseValueType baseValue) const override
    {
        actualValue.set(baseValue.redF(), baseValue.greenF(), baseValue.blueF(), baseValue.alphaF());
        return true;
    }

    /*
    bool toStrImpl(QString& str) const override
    {
        str = CEGUIUtils::stringToQString(CEGUI::PropertyHelper<CEGUI::Colour>().toString(value()));
        return true;
    }

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override
    {
        try
        {
            return setValue(CEGUI::PropertyHelper<CEGUI::Colour>().fromString(CEGUIUtils::qStringToString(str)), reason);
        }
        catch (...)
        {
        }

        return false;
    }
    */
};

P_PROPERTY_DECL_ALL_OPERATORS(QtnPropertyColourBase, CEGUI::Colour)

class QtnPropertyColourCallback : public QtnSinglePropertyCallback<QtnPropertyColourBase>
{
    Q_OBJECT

    QtnPropertyColourCallback(const QtnPropertyColourCallback &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyColourCallback(QObject *parent = nullptr)
        : QtnSinglePropertyCallback<QtnPropertyColourBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(
        QtnPropertyColourCallback, QtnPropertyColourBase)
};

class QtnPropertyColour : public QtnSinglePropertyValue<QtnPropertyColourBase>
{
    Q_OBJECT

    QtnPropertyColour(const QtnPropertyColour &other) Q_DECL_EQ_DELETE;

public:
    Q_INVOKABLE explicit QtnPropertyColour(QObject *parent = nullptr)
        : QtnSinglePropertyValue<QtnPropertyColourBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyColour, QtnPropertyColourBase)
};

#endif // QTNPROPERTYCOLOUR_H
