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

#include "PropertyConnector.h"

#include "PropertyBase.h"
#include "Property.h"
#include "QObjectPropertySet.h"
#include "IQtnPropertyStateProvider.h"

QtnPropertyConnector::QtnPropertyConnector(QtnPropertyBase *property)
	: QObject(property)
	, property(property)
	, object(nullptr)
	, ignoreStateChangeCounter(0)
{
	Q_ASSERT(nullptr != property);
	Q_ASSERT(nullptr == property->getConnector());
	property->setConnector(this);
}

void QtnPropertyConnector::connectProperty(
	QObject *object, const QMetaProperty &metaProperty)
{
	this->object = object;
	this->metaProperty = metaProperty;
	auto metaObject = this->metaObject();
	if (metaProperty.hasNotifySignal())
	{
		auto slot =
			metaObject->method(metaObject->indexOfSlot("onValueChanged()"));
		QObject::connect(object, metaProperty.notifySignal(), this, slot);
	}

	auto stateProvider = dynamic_cast<IQtnPropertyStateProvider *>(object);

	if (nullptr != stateProvider)
	{
		auto srcMetaObject = object->metaObject();
		auto signal = srcMetaObject->method(
			srcMetaObject->indexOfSignal("modifiedSetChanged()"));
		if (signal.isValid())
		{
			auto slot = metaObject->method(
				metaObject->indexOfSlot("onModifiedSetChanged()"));
			QObject::connect(object, signal, this, slot);
		}

		signal = srcMetaObject->method(srcMetaObject->indexOfSignal(
			"propertyStateChanged(QMetaProperty)"));
		if (signal.isValid())
		{
			auto slot = metaObject->method(metaObject->indexOfSlot(
				"onPropertyStateChanged(QMetaProperty)"));
			QObject::connect(object, signal, this, slot);
		}
	} else
	{
		property->switchState(
			QtnPropertyStateResettable, metaProperty.isResettable());
	}
}

void QtnPropertyConnector::updatePropertyState()
{
	if (!property || !object || !metaProperty.isValid())
	{
		return;
	}

	auto stateProvider = dynamic_cast<IQtnPropertyStateProvider *>(object);
	if (!stateProvider)
	{
		return;
	}

	ignoreStateChangeCounter++;
	stateProvider->setPropertyState(metaProperty, property->stateLocal());
	ignoreStateChangeCounter--;
}

bool QtnPropertyConnector::isResettablePropertyValue() const
{
	return metaProperty.isResettable();
}

void QtnPropertyConnector::resetPropertyValue(QtnPropertyChangeReason reason)
{
	if (nullptr != object && nullptr != property && metaProperty.isResettable())
	{
		if (property->isResettable())
		{
			reason |= QtnPropertyChangeReasonResetValue;

			emit property->propertyWillChange(reason, nullptr, 0);
			metaProperty.reset(object);
			emit property->propertyDidChange(reason);
		}
	}
}

void QtnPropertyConnector::onValueChanged()
{
	if (nullptr != property)
	{
		property->postUpdateEvent(QtnPropertyChangeReasonNewValue, 20);
	}
}

void QtnPropertyConnector::onPropertyStateChanged(
	const QMetaProperty &metaProperty)
{
	if (ignoreStateChangeCounter == 0 && nullptr != property &&
		metaProperty.propertyIndex() == this->metaProperty.propertyIndex())
	{
		onModifiedSetChanged();
	}
}

void QtnPropertyConnector::onModifiedSetChanged()
{
	if (nullptr != property)
	{
		auto stateProvider = dynamic_cast<IQtnPropertyStateProvider *>(object);

		if (nullptr == stateProvider)
			return;

		QtnPropertyState state;
		state = stateProvider->getPropertyState(metaProperty);
		state |= qtnPropertyStateToAdd(metaProperty);
		state.setFlag(QtnPropertyStateCollapsed, property->isCollapsed());
		state.setFlag(QtnPropertyStateResettable, metaProperty.isResettable());

		property->setState(state);
	}
}
