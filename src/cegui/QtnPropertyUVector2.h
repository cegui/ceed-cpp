#ifndef PROPERTYUVECTOR2_H
#define PROPERTYUVECTOR2_H

#include "src/cegui/CEGUIUtils.h"
#include "3rdParty/QtnProperty/Core/Auxiliary/PropertyTemplates.h"
#include "3rdParty/QtnProperty/PropertyWidget/Delegates/Utils/PropertyDelegateMisc.h"
#include <CEGUI/UVector.h>

//???two UDim properties? UDim may have own widget! operators <<, >> too!
//only if UDim editor is in-line. 2-level expansion is not user friendly.

class QtnPropertyUVector2Base: public QtnSinglePropertyBase<CEGUI::UVector2>
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

class QTN_PW_EXPORT QtnPropertyDelegateUVector2: public QtnPropertyDelegateTypedEx<QtnPropertyUVector2Base>
{
    Q_DISABLE_COPY(QtnPropertyDelegateUVector2)

public:
    QtnPropertyDelegateUVector2(QtnPropertyUVector2Base& owner);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::UVector2);

#endif // PROPERTYUVECTOR2_H
