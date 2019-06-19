#ifndef PROPERTYUBox_H
#define PROPERTYUBox_H

#include "src/cegui/CEGUIUtils.h"
#include "QtnProperty/Auxiliary/PropertyTemplates.h"
#include "QtnProperty/Delegates/Utils/PropertyDelegateMisc.h"
#include <CEGUI/UDim.h>

class QtnPropertyUBoxBase: public QtnSinglePropertyBase<CEGUI::UBox>
{
    Q_OBJECT
    QtnPropertyUBoxBase(const QtnPropertyUBoxBase& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUBoxBase(QObject* parent);

protected:
    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyUBoxBase)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyUBoxBase, CEGUI::UBox)

class QtnPropertyUBoxCallback: public QtnSinglePropertyCallback<QtnPropertyUBoxBase>
{
    Q_OBJECT
    QtnPropertyUBoxCallback(const QtnPropertyUBoxCallback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUBoxCallback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyUBoxBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUBoxCallback, QtnPropertyUBoxBase)
};

class QtnPropertyUBox: public QtnSinglePropertyValue<QtnPropertyUBoxBase>
{
    Q_OBJECT
    QtnPropertyUBox(const QtnPropertyUBox& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyUBox(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyUBoxBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyUBox, QtnPropertyUBoxBase)
};

class QtnPropertyDelegateUBox: public QtnPropertyDelegateTypedEx<QtnPropertyUBoxBase>
{
    Q_DISABLE_COPY(QtnPropertyDelegateUBox)

public:
    QtnPropertyDelegateUBox(QtnPropertyUBoxBase& owner);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::UBox);

#endif // PROPERTYUBox_H
