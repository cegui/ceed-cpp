#ifndef PROPERTYUSize_H
#define PROPERTYUSize_H

#include "src/cegui/CEGUIUtils.h"
#include "3rdParty/QtnProperty/Core/Auxiliary/PropertyTemplates.h"
#include "3rdParty/QtnProperty/PropertyWidget/Delegates/Utils/PropertyDelegateMisc.h"
#include <CEGUI/USize.h>

class QtnPropertyUSizeBase: public QtnSinglePropertyBase<CEGUI::USize>
{
    Q_OBJECT
    QtnPropertyUSizeBase(const QtnPropertyUSizeBase& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUSizeBase(QObject* parent);

protected:
    bool fromStrImpl(const QString& str) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyUSizeBase)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyUSizeBase, CEGUI::USize)

class QtnPropertyUSizeCallback: public QtnSinglePropertyCallback<QtnPropertyUSizeBase>
{
    Q_OBJECT
    QtnPropertyUSizeCallback(const QtnPropertyUSizeCallback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUSizeCallback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyUSizeBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUSizeCallback, QtnPropertyUSizeBase)
};

class QtnPropertyUSize: public QtnSinglePropertyValue<QtnPropertyUSizeBase>
{
    Q_OBJECT
    QtnPropertyUSize(const QtnPropertyUSize& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUSize(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyUSizeBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUSize, QtnPropertyUSizeBase)
};

class QtnPropertyDelegateUSize: public QtnPropertyDelegateTypedEx<QtnPropertyUSizeBase>
{
    Q_DISABLE_COPY(QtnPropertyDelegateUSize)

public:
    QtnPropertyDelegateUSize(QtnPropertyUSizeBase& owner);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::USize);

#endif // PROPERTYUSize_H
