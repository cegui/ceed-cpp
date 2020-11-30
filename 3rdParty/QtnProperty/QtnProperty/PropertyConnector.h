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

#pragma once

#include "Config.h"
#include "Auxiliary/PropertyAux.h"

#include <QMetaProperty>
#include <QObject>

class QtnPropertyBase;
class QtnPropertySet;

class QTN_IMPORT_EXPORT QtnPropertyConnector : public QObject
{
	Q_OBJECT

public:
	explicit QtnPropertyConnector(QtnPropertyBase *property);

	void connectProperty(QObject *object, const QMetaProperty &metaProperty);
	void updatePropertyState();

	bool isResettablePropertyValue() const;
	void resetPropertyValue(QtnPropertyChangeReason reason);

	inline QObject *getObject() const;
	inline const QMetaProperty &getMetaProperty() const;

private slots:
	void onValueChanged();
	void onModifiedSetChanged();
	void onPropertyStateChanged(const QMetaProperty &metaProperty);

private:
	QtnPropertyBase *property;
	QObject *object;
	QMetaProperty metaProperty;
	unsigned ignoreStateChangeCounter;
};

QObject *QtnPropertyConnector::getObject() const
{
	return object;
}

const QMetaProperty &QtnPropertyConnector::getMetaProperty() const
{
	return metaProperty;
}
