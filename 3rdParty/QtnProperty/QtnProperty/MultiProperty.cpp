/*******************************************************************************
Copyright (c) 2015-2019 Alexandra Cherdantseva <neluhus.vagus@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#include "MultiProperty.h"

#include "Install.h"
#include "Property.h"
#include "PropertySet.h"
#include "PropertyConnector.h"
#include "Delegates/PropertyDelegateFactory.h"
#include "Utils/QtnConnections.h"

#include <QStyleOption>
#include <QKeyEvent>
#include <QLineEdit>

struct QtnMultiPropertyDelegate::PropertyToEdit
{
	QtnMultiProperty *owner;
	QtnProperty *property;
	QtnConnections connections;
};

QtnMultiProperty::QtnMultiProperty(
	const QMetaObject *propertyMetaObject, QObject *parent)
	: QtnProperty(parent)
	, mPropertyMetaObject(propertyMetaObject)
	, m_subPropertyUpdates(0)
	, edited(false)
	, calculateMultipleValues(true)
	, multipleValues(false)
{
	setDelegateInfo(QtnPropertyDelegateInfo());
}

QtnMultiProperty::~QtnMultiProperty()
{
	for (auto property : properties)
	{
		QObject::disconnect(property, &QtnProperty::propertyValueAccept, this,
			&QtnMultiProperty::onPropertyValueAccept);
		QObject::disconnect(property, &QtnPropertyBase::propertyWillChange,
			this, &QtnMultiProperty::onPropertyWillChange);
		QObject::disconnect(property, &QtnPropertyBase::propertyDidChange, this,
			&QtnMultiProperty::onPropertyDidChange);
	}
}

const QMetaObject *QtnMultiProperty::propertyMetaObject() const
{
	return mPropertyMetaObject;
}

void QtnMultiProperty::addProperty(QtnProperty *property, bool own)
{
	Q_ASSERT(nullptr != property);
	Q_ASSERT(nullptr != mPropertyMetaObject->cast(property));

	if (own)
		property->setParent(this);

	if (properties.end() !=
		std::find(properties.begin(), properties.end(), property))
	{
		return;
	}

	properties.push_back(property);

	if (property->isCollapsed())
		collapse();
	updateStateFrom(property);

	QObject::connect(property, &QtnProperty::propertyValueAccept, this,
		&QtnMultiProperty::onPropertyValueAccept);
	QObject::connect(property, &QtnPropertyBase::propertyWillChange, this,
		&QtnMultiProperty::onPropertyWillChange);
	QObject::connect(property, &QtnPropertyBase::propertyDidChange, this,
		&QtnMultiProperty::onPropertyDidChange);
}

void QtnMultiProperty::doReset(QtnPropertyChangeReason reason)
{
	Q_ASSERT(reason & QtnPropertyChangeReasonResetValue);

	m_subPropertyUpdates++;
	emit propertyWillChange(reason, nullptr, 0);

	for (auto property : properties)
	{
		property->reset(reason);
	}

	emit propertyDidChange(reason);
	m_subPropertyUpdates--;

	updateMultipleState(true);
}

QString QtnMultiProperty::getMultiValuePlaceholder()
{
	return tr("(Multiple Values)");
}

QMetaProperty QtnMultiProperty::getMetaProperty() const
{
	Q_ASSERT(!properties.empty());

	auto connector = properties.at(0)->getConnector();
	Q_ASSERT(nullptr != connector);

	return connector->getMetaProperty();
}

bool QtnMultiProperty::hasMultipleValues() const
{
	if (calculateMultipleValues)
	{
		QString dummy;
		toStrImpl(dummy);
	}

	return multipleValues;
}

void QtnMultiProperty::onPropertyValueAccept(
	QtnPropertyValuePtr valueToAccept, bool *accept)
{
	if (m_subPropertyUpdates)
		return;
	emit propertyValueAccept(valueToAccept, accept);
}

void QtnMultiProperty::onPropertyWillChange(
	QtnPropertyChangeReason reason, QtnPropertyValuePtr newValue, int typeId)
{
	if (m_subPropertyUpdates)
		return;
	emit propertyWillChange(reason, newValue, typeId);
}

void QtnMultiProperty::onPropertyDidChange(QtnPropertyChangeReason reason)
{
	if (m_subPropertyUpdates)
		return;

	Q_ASSERT(nullptr != qobject_cast<QtnProperty *>(sender()));
	auto changedProperty = static_cast<QtnProperty *>(sender());
	if (edited && (reason & QtnPropertyChangeReasonEdit) &&
		(reason & QtnPropertyChangeReasonValue))
	{
		auto value = changedProperty->valueAsVariant();

		auto singleReason = reason & ~QtnPropertyChangeReasonMultiEdit;

		m_subPropertyUpdates++;
		for (auto property : properties)
		{
			if (property != changedProperty && property->isEditableByUser())
			{
				property->fromVariant(value, singleReason);
			}
		}
		m_subPropertyUpdates--;
	}

	if (reason & (QtnPropertyChangeReasonState | QtnPropertyChangeReasonValue))
	{
		updateStateFrom(changedProperty);
		updateMultipleState(true);
	}

	emit propertyDidChange(reason);
}

void QtnMultiProperty::updatePropertyState()
{
	QtnProperty::updatePropertyState();

	if (m_subPropertyUpdates)
	{
		return;
	}

	if (!stateLocal().testFlag(QtnPropertyStateUnlockable))
	{
		return;
	}

	bool isImmutable = stateLocal().testFlag(QtnPropertyStateImmutable);
	m_subPropertyUpdates++;
	for (auto property : properties)
	{
		if (property->stateLocal().testFlag(QtnPropertyStateUnlockable))
		{
			property->switchState(QtnPropertyStateImmutable, isImmutable);
		}
	}
	m_subPropertyUpdates--;
}

bool QtnMultiProperty::loadImpl(QDataStream &stream)
{
	for (auto property : properties)
	{
		QByteArray propertyData;
		stream >> propertyData;
		QDataStream propertyStream(&propertyData, QIODevice::ReadOnly);

		if (!property->load(propertyStream))
			return false;
	}

	return true;
}

bool QtnMultiProperty::saveImpl(QDataStream &stream) const
{
	for (auto property : properties)
	{
		QByteArray propertyData;
		QDataStream propertyStream(&propertyData, QIODevice::WriteOnly);

		if (!property->save(propertyStream))
			return false;

		stream << propertyData;
	}

	return true;
}

void QtnMultiProperty::masterPropertyWillChange(QtnPropertyChangeReason reason)
{
	if (m_subPropertyUpdates)
		return;
	QtnProperty::masterPropertyWillChange(reason);
}

void QtnMultiProperty::masterPropertyDidChange(QtnPropertyChangeReason reason)
{
	if (m_subPropertyUpdates)
		return;
	if (reason & (QtnPropertyChangeReasonState | QtnPropertyChangeReasonValue))
	{
		updateMultipleState(true);
	}
	QtnProperty::masterPropertyDidChange(reason);
}

bool QtnMultiProperty::fromStrImpl(
	const QString &str, QtnPropertyChangeReason reason)
{
	m_subPropertyUpdates++;
	emit propertyWillChange(reason, nullptr, 0);
	int okCount = 0;
	for (auto property : properties)
	{
		if (property->fromStr(str, reason))
			okCount++;
	}
	emit propertyDidChange(reason);
	m_subPropertyUpdates--;
	return okCount > 0;
}

bool QtnMultiProperty::toStrImpl(QString &str) const
{
	if (calculateMultipleValues)
	{
		auto thiz = const_cast<QtnMultiProperty *>(this);
		thiz->calculateMultipleValues = false;

		size_t sameCount = 0;
		QString temp;

		for (auto property : properties)
		{
			if (!property->toStr(str))
				str.clear();

			if (sameCount == 0)
				temp = str;

			if (sameCount == 0 || str == temp)
				sameCount++;
		}

		thiz->multipleValues = (sameCount != properties.size());
	} else if (!multipleValues)
	{
		if (properties.empty() || !properties.at(0)->toStr(str))
			str.clear();
	}

	if (multipleValues)
		str.clear();

	return true;
}

bool QtnMultiProperty::fromVariantImpl(
	const QVariant &var, QtnPropertyChangeReason reason)
{
	int okCount = 0;
	m_subPropertyUpdates++;
	emit propertyWillChange(reason, nullptr, 0);
	if (var.userType() == qMetaTypeId<QtnMultiVariant>())
	{
		auto values = var.value<QtnMultiVariant>().values;
		auto count = properties.size();
		auto varCount = size_t(values.count());

		if (count > varCount)
			count = varCount;

		for (size_t i = 0; i < count; i++)
		{
			QtnProperty *property = properties.at(i);

			if (property->fromVariant(values.at(int(i)), reason))
				okCount++;
		}
	} else
	{
		for (auto property : properties)
		{
			if (property->fromVariant(var, reason))
				okCount++;
		}
	}
	emit propertyDidChange(reason);
	m_subPropertyUpdates--;
	return okCount > 0;
}

bool QtnMultiProperty::toVariantImpl(QVariant &var) const
{
	QtnMultiVariant multiVariant;

	for (auto property : properties)
	{
		if (!property->toVariant(var))
			var.clear();

		multiVariant.values.push_back(var);
	}

	var.setValue(multiVariant);

	return true;
}

void QtnMultiProperty::updateStateFrom(QtnProperty *source)
{
	static const QtnPropertyState unchangedState(QtnPropertyStateMultiValue |
		QtnPropertyStateCollapsed | QtnPropertyStateModifiedValue);
	auto state = stateLocal() & unchangedState;
	state |= source->stateLocal() & ~unchangedState;

	state &= ~(QtnPropertyStateImmutable | QtnPropertyStateResettable |
		QtnPropertyStateInvisible | QtnPropertyStateUnlockable);

	size_t unlockableCount = 0;
	for (auto property : properties)
	{
		auto childState = property->stateLocal();
		if (childState.testFlag(QtnPropertyStateInvisible))
		{
			state |= QtnPropertyStateInvisible;
		}

		if (childState.testFlag(QtnPropertyStateImmutable))
		{
			state |= QtnPropertyStateImmutable;
		}

		if (childState.testFlag(QtnPropertyStateResettable))
		{
			state |= QtnPropertyStateResettable;
		}

		if (childState.testFlag(QtnPropertyStateUnlockable))
		{
			unlockableCount++;
		}
	}

	if (unlockableCount == properties.size())
	{
		state |= QtnPropertyStateUnlockable;
	}

	m_subPropertyUpdates++;
	setState(state);
	m_subPropertyUpdates--;
}

void QtnMultiProperty::updateMultipleState(bool force)
{
	if (force)
		calculateMultipleValues = true;

	bool multipleValues = hasMultipleValues();
	auto state = stateLocal() & ~QtnPropertyStateModifiedValue;

	state.setFlag(QtnPropertyStateMultiValue, multipleValues);

	for (auto property : properties)
	{
		if (!property->valueIsDefault())
		{
			state |= QtnPropertyStateModifiedValue;
			break;
		}
	}

	m_subPropertyUpdates++;

	setState(state);

	m_subPropertyUpdates--;
}

QtnMultiPropertyDelegate::QtnMultiPropertyDelegate(QtnMultiProperty &owner)
	: Inherited(owner)
{
	owner.updateMultipleState(true);
}

void QtnMultiPropertyDelegate::init()
{
	Q_ASSERT(superDelegates.empty());

	auto &properties = owner().properties;
	superDelegates.reserve(properties.size());
	for (auto property : properties)
	{
		auto delegate = factory()->createDelegate(*property);
		delegate->setStateProperty(&owner());
		superDelegates.emplace_back(delegate);
	}
}

QtnMultiPropertyDelegate::~QtnMultiPropertyDelegate()
{
	m_subProperties.clear();
}

void QtnMultiPropertyDelegate::Register(QtnPropertyDelegateFactory &factory)
{
	factory.registerDelegateDefault(&QtnMultiProperty::staticMetaObject,
		&qtnCreateDelegate<QtnMultiPropertyDelegate, QtnMultiProperty>,
		"MultiProperty");
}

void QtnMultiPropertyDelegate::onEditedPropertyDestroyed(PropertyToEdit *data)
{
	Q_ASSERT(nullptr != data);
	data->owner = nullptr;
	data->property = nullptr;
	data->connections.clear();
}

void QtnMultiPropertyDelegate::onEditorDestroyed(PropertyToEdit *data)
{
	auto multiProperty = data->owner;
	if (multiProperty)
	{
		multiProperty->edited = false;
	}

	delete data;
}

void QtnMultiPropertyDelegate::applyAttributesImpl(
	const QtnPropertyDelegateInfo &info)
{
	for (auto &delegate : superDelegates)
	{
		QtnPropertyDelegateInfo mergedInfo;
		mergedInfo.attributes = info.attributes;
		auto delegateInfoPtr = delegate->propertyImmutable()->delegateInfo();
		if (delegateInfoPtr)
		{
			mergedInfo.name = delegateInfoPtr->name;
			auto &attributes = delegateInfoPtr->attributes;
			for (auto it = attributes.cbegin(); it != attributes.cend(); ++it)
			{
				auto rootIt = mergedInfo.attributes.find(it.key());
				if (rootIt != mergedInfo.attributes.end())
					continue;

				mergedInfo.attributes[it.key()] = it.value();
			}
		}
		delegate->applyAttributes(mergedInfo);

		for (int i = 0, count = delegate->subPropertyCount(); i < count; ++i)
		{
			auto property = delegate->subProperty(i);
			auto it = std::find_if(m_subProperties.begin(),
				m_subProperties.end(),
				[property](const QScopedPointer<QtnPropertyBase> &a) -> bool {
					return property->propertyMetaObject() ==
						a->propertyMetaObject() &&
						property->displayName() == a->displayName();
				});

			auto subSet = property->asPropertySet();
			if (subSet)
			{
				QtnPropertySet *multiSet;

				if (it == m_subProperties.end())
				{
					multiSet = new QtnPropertySet(
						subSet->childrenOrder(), subSet->compareFunc());
					multiSet->setName(subSet->name());
					multiSet->setDisplayName(subSet->displayName());
					multiSet->setDescription(subSet->description());
					multiSet->setId(subSet->id());
					multiSet->setState(subSet->stateLocal());

					addSubProperty(multiSet);
				} else
				{
					multiSet = it->data()->asPropertySet();
				}

				qtnPropertiesToMultiSet(multiSet, subSet, false);
			} else
			{
				QtnMultiProperty *multiProperty;

				if (it == m_subProperties.end())
				{
					multiProperty =
						new QtnMultiProperty(property->metaObject());
					multiProperty->setName(property->name());
					multiProperty->setDisplayName(property->displayName());
					multiProperty->setDescription(property->description());
					multiProperty->setId(property->id());

					addSubProperty(multiProperty);
				} else
				{
					Q_ASSERT(qobject_cast<QtnMultiProperty *>(it->data()));
					multiProperty = static_cast<QtnMultiProperty *>(it->data());
				}

				multiProperty->addProperty(property->asProperty(), false);
			}
		}
	}
}

void QtnMultiPropertyDelegate::createSubItemsImpl(
	QtnDrawContext &context, QList<QtnSubItem> &subItems)
{
	Q_ASSERT(!superDelegates.empty());
	superDelegates.at(0)->createSubItems(context, subItems);
	for (auto &item : subItems)
	{
		if (nullptr == item.eventHandler)
			continue;

		auto oldEventHandler = item.eventHandler;
		item.eventHandler = [oldEventHandler, this](QtnEventContext &context,
								const QtnSubItem &item,
								QtnPropertyToEdit *toEdit) -> bool //
		{
			if (!oldEventHandler(context, item, toEdit))
				return false;

			if (!toEdit->isValid() || toEdit->property() == property() ||
				!property()->isEditableByUser())
			{
				return true;
			}

			QtnPropertyToEdit oldToEdit(*toEdit);
			toEdit->setup(property(),
				[this, oldToEdit]() -> QWidget * //
				{
					QtnMultiPropertyDelegate *thiz = this;
					auto &p = thiz->owner();
					auto propertyToEdit = p.properties.at(0);
					auto data = new PropertyToEdit;
					data->owner = &p;
					data->property = propertyToEdit;
					p.edited = true;

					using namespace std::placeholders;

					data->connections.emplace_back(
						QObject::connect(propertyToEdit, &QObject::destroyed,
							std::bind(&QtnMultiPropertyDelegate::
										  onEditedPropertyDestroyed,
								data)));
					auto editor = oldToEdit.createEditor();
					if (editor)
					{
						data->connections.emplace_back(QObject::connect(editor,
							&QObject::destroyed,
							std::bind(
								&QtnMultiPropertyDelegate::onEditorDestroyed,
								data)));
					} else
					{
						onEditorDestroyed(data);
					}
					return editor;
				});
			return true;
		};
	}
}

void qtnPropertiesToMultiSet(
	QtnPropertySet *target, QtnPropertySet *source, bool takeOwnership)
{
	Q_ASSERT(target);
	Q_ASSERT(source);

	auto &targetProperties = target->childProperties();
	for (auto property : source->childProperties())
	{
		auto it = std::find_if(targetProperties.begin(), targetProperties.end(),
			[property](const QtnPropertyBase *targetProperty) -> bool {
				return property->propertyMetaObject() ==
					targetProperty->propertyMetaObject() &&
					property->displayName() == targetProperty->displayName();
			});

		auto subSet = property->asPropertySet();
		if (subSet)
		{
			QtnPropertySet *multiSet;

			if (it == targetProperties.end())
			{
				multiSet = new QtnPropertySet(
					subSet->childrenOrder(), subSet->compareFunc());
				multiSet->setName(subSet->name());
				multiSet->setDisplayName(subSet->displayName());
				multiSet->setDescription(subSet->description());
				multiSet->setId(subSet->id());
				multiSet->setState(subSet->stateLocal());

				target->addChildProperty(multiSet, true);
			} else
			{
				multiSet = (*it)->asPropertySet();
			}

			qtnPropertiesToMultiSet(multiSet, subSet, takeOwnership);
		} else
		{
			QtnMultiProperty *multiProperty;

			if (it == targetProperties.end())
			{
				multiProperty = new QtnMultiProperty(property->metaObject());
				multiProperty->setName(property->name());
				multiProperty->setDisplayName(property->displayName());
				multiProperty->setDescription(property->description());
				multiProperty->setId(property->id());

				target->addChildProperty(multiProperty, true);
			} else
			{
				Q_ASSERT(qobject_cast<QtnMultiProperty *>(*it));
				multiProperty = static_cast<QtnMultiProperty *>(*it);
			}

			multiProperty->addProperty(property->asProperty(), takeOwnership);
		}
	}
	if (takeOwnership)
		source->clearChildProperties();
}
