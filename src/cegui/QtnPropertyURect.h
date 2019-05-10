#ifndef PROPERTYURect_H
#define PROPERTYURect_H

#include "src/cegui/CEGUIUtils.h"
#include "3rdParty/QtnProperty/Core/Auxiliary/PropertyTemplates.h"
#include "3rdParty/QtnProperty/PropertyWidget/Delegates/Utils/PropertyDelegateMisc.h"
#include <CEGUI/URect.h>

class QtnPropertyURectBase: public QtnSinglePropertyBase<CEGUI::URect>
{
    Q_OBJECT
    QtnPropertyURectBase(const QtnPropertyURectBase& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyURectBase(QObject* parent);

protected:
    bool fromStrImpl(const QString& str) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyURectBase)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyURectBase, CEGUI::URect)

class QtnPropertyURectCallback: public QtnSinglePropertyCallback<QtnPropertyURectBase>
{
    Q_OBJECT
    QtnPropertyURectCallback(const QtnPropertyURectCallback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyURectCallback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyURectBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyURectCallback, QtnPropertyURectBase)
};

class QtnPropertyURect: public QtnSinglePropertyValue<QtnPropertyURectBase>
{
    Q_OBJECT
    QtnPropertyURect(const QtnPropertyURect& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyURect(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyURectBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyURect, QtnPropertyURectBase)
};

class QtnPropertyDelegateURect: public QtnPropertyDelegateTypedEx<QtnPropertyURectBase>
{
    Q_DISABLE_COPY(QtnPropertyDelegateURect)

public:
    QtnPropertyDelegateURect(QtnPropertyURectBase& owner);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::URect);

#endif // PROPERTYURect_H
