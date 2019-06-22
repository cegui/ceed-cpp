#ifndef PROPERTYUVector3_H
#define PROPERTYUVector3_H

#include "src/cegui/CEGUIUtils.h"
#include "src/cegui/QtnPropertyUDim.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include "QtnProperty/Delegates/Utils/PropertyDelegateMisc.h"
#include "QtnProperty/StructPropertyBase.h"
#include <CEGUI/UVector.h>

class QtnPropertyUVector3Base: public QtnStructPropertyBase<CEGUI::UVector3, QtnPropertyUDimCallback>
{
    Q_OBJECT
    QtnPropertyUVector3Base(const QtnPropertyUVector3Base& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector3Base(QObject* parent);

    QtnProperty* createXProperty();
    QtnProperty* createYProperty();
    QtnProperty* createZProperty();

protected:
    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyUVector3Base)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyUVector3Base, CEGUI::UVector3)

class QtnPropertyUVector3Callback: public QtnSinglePropertyCallback<QtnPropertyUVector3Base>
{
    Q_OBJECT
    QtnPropertyUVector3Callback(const QtnPropertyUVector3Callback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector3Callback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyUVector3Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUVector3Callback, QtnPropertyUVector3Base)
};

class QtnPropertyUVector3: public QtnSinglePropertyValue<QtnPropertyUVector3Base>
{
    Q_OBJECT
    QtnPropertyUVector3(const QtnPropertyUVector3& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUVector3(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyUVector3Base>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUVector3, QtnPropertyUVector3Base)
};

class QtnPropertyDelegateUVector3: public QtnPropertyDelegateTypedEx<QtnPropertyUVector3Base>
{
    Q_DISABLE_COPY(QtnPropertyDelegateUVector3)

public:
    QtnPropertyDelegateUVector3(QtnPropertyUVector3Base& owner);

    static void Register(QtnPropertyDelegateFactory& factory);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::UVector3);

#endif // PROPERTYUVector3_H
