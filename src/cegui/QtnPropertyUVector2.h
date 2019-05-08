#ifndef PROPERTYUVECTOR2_H
#define PROPERTYUVECTOR2_H

#include "3rdParty/QtnProperty/Core/Auxiliary/PropertyTemplates.h"
#include <CEGUI/UVector.h>

//???two UDim properties? UDim may have own widget! operators <<, >> too!

class QTN_PE_CORE_EXPORT QtnPropertyUVector2Base: public QtnSinglePropertyBase<CEGUI::UVector2>
{
    Q_OBJECT
    QtnPropertyUVector2Base(const QtnPropertyUVector2Base& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector2Base(QObject* parent);

protected:
    // string conversion implementation
    bool fromStrImpl(const QString& str) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyUVector2Base)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyUVector2Base, CEGUI::UVector2)

QTN_PE_CORE_EXPORT QtnProperty* qtnCreateXScaleProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty);
QTN_PE_CORE_EXPORT QtnProperty* qtnCreateXOffsetProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty);
QTN_PE_CORE_EXPORT QtnProperty* qtnCreateYScaleProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty);
QTN_PE_CORE_EXPORT QtnProperty* qtnCreateYOffsetProperty(QObject* parent, QtnPropertyUVector2Base* mainProperty);

class QTN_PE_CORE_EXPORT QtnPropertyUVector2Callback: public QtnSinglePropertyCallback<QtnPropertyUVector2Base>
{
    Q_OBJECT
    QtnPropertyUVector2Callback(const QtnPropertyUVector2Callback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector2Callback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyUVector2Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUVector2Callback, QtnPropertyUVector2Base)
};

class QTN_PE_CORE_EXPORT QtnPropertyUVector2: public QtnSinglePropertyValue<QtnPropertyUVector2Base>
{
    Q_OBJECT
    QtnPropertyUVector2(const QtnPropertyUVector2& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector2(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyUVector2Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUVector2, QtnPropertyUVector2Base)
};

QDataStream& operator <<(QDataStream& stream, const CEGUI::UVector2& value);
QDataStream& operator >>(QDataStream& stream, CEGUI::UVector2& value);

Q_DECLARE_METATYPE(CEGUI::UVector2);

#endif // PROPERTYUVECTOR2_H
