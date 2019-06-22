#ifndef PROPERTYUVECTOR2_H
#define PROPERTYUVECTOR2_H

#include "src/cegui/CEGUIUtils.h"
#include "src/cegui/QtnPropertyUDim.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include "QtnProperty/Delegates/Utils/PropertyDelegateMisc.h"
#include "QtnProperty/StructPropertyBase.h"
#include <CEGUI/UVector.h>

class QtnPropertyUVector2Base: public QtnStructPropertyBase<CEGUI::UVector2, QtnPropertyUDimCallback>
{
    Q_OBJECT
    QtnPropertyUVector2Base(const QtnPropertyUVector2Base& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector2Base(QObject* parent);

    QtnProperty* createXProperty();
    QtnProperty* createYProperty();

protected:
    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyUVector2Base)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyUVector2Base, CEGUI::UVector2)

class QtnPropertyUVector2Callback: public QtnSinglePropertyCallback<QtnPropertyUVector2Base>
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

class QtnPropertyUVector2: public QtnSinglePropertyValue<QtnPropertyUVector2Base>
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

class QtnPropertyDelegateUVector2: public QtnPropertyDelegateTypedEx<QtnPropertyUVector2Base>
{
    Q_DISABLE_COPY(QtnPropertyDelegateUVector2)

public:
    QtnPropertyDelegateUVector2(QtnPropertyUVector2Base& owner);

    static void Register(QtnPropertyDelegateFactory& factory);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::UVector2);

#endif // PROPERTYUVECTOR2_H
