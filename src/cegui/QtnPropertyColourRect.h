#ifndef QTNPROPERTYCOLOURRECT_H
#define QTNPROPERTYCOLOURRECT_H

#include <src/cegui/QtnPropertyColour.h>
#include "QtnProperty/Delegates/Utils/PropertyDelegateMisc.h"
#include <CEGUI/ColourRect.h>

class QtnPropertyColourRectBase: public QtnStructPropertyBase<CEGUI::ColourRect, QtnPropertyColourCallback>
{
    Q_OBJECT

    QtnPropertyColourRectBase(const QtnPropertyColourRectBase& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyColourRectBase(QObject* parent);

    QtnProperty* createTopLeftProperty();
    QtnProperty* createTopRightProperty();
    QtnProperty* createBottomLeftProperty();
    QtnProperty* createBottomRightProperty();

protected:

    bool fromStrImpl(const QString& str, QtnPropertyChangeReason reason) override;
    bool toStrImpl(QString& str) const override;

    P_PROPERTY_DECL_MEMBER_OPERATORS(QtnPropertyColourRectBase)
};

P_PROPERTY_DECL_EQ_OPERATORS(QtnPropertyColourRectBase, CEGUI::ColourRect)

class QtnPropertyColourRectCallback: public QtnSinglePropertyCallback<QtnPropertyColourRectBase>
{
    Q_OBJECT
    QtnPropertyColourRectCallback(const QtnPropertyColourRectCallback& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyColourRectCallback(QObject* parent)
        : QtnSinglePropertyCallback<QtnPropertyColourRectBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyColourRectCallback, QtnPropertyColourRectBase)
};

class QtnPropertyColourRect: public QtnSinglePropertyValue<QtnPropertyColourRectBase>
{
    Q_OBJECT
    QtnPropertyColourRect(const QtnPropertyColourRect& other) Q_DECL_EQ_DELETE;

public:
    explicit QtnPropertyColourRect(QObject* parent)
        : QtnSinglePropertyValue<QtnPropertyColourRectBase>(parent)
    {
    }

    P_PROPERTY_DECL_MEMBER_OPERATORS2(QtnPropertyColourRect, QtnPropertyColourRectBase)
};

class QtnPropertyDelegateColourRect: public QtnPropertyDelegateTypedEx<QtnPropertyColourRectBase>
{
    Q_DISABLE_COPY(QtnPropertyDelegateColourRect)

public:
    QtnPropertyDelegateColourRect(QtnPropertyColourRectBase& owner);

    static void Register(QtnPropertyDelegateFactory& factory);

protected:
    QWidget* createValueEditorImpl(QWidget* parent, const QRect& rect, QtnInplaceInfo* inplaceInfo = nullptr) override;
    bool propertyValueToStrImpl(QString& strValue) const override;
};

Q_DECLARE_METATYPE(CEGUI::ColourRect);

#endif // QTNPROPERTYCOLOURRECT_H
