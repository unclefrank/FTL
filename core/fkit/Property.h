/*
 * Copyright (C) 2007-2013 Frank Mertens.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef FKIT_PROPERTY_H
#define FKIT_PROPERTY_H

#include "Format.h"
#include "Map.h"

namespace fkit
{

template<class Value>
class Callback: public Object {
public:
	virtual void invoke(Value value) = 0;
};

template<class Recipient, class Value>
class Slot: public Callback<Value>
{
public:
	typedef void (Recipient::* Method)(Value);

	Slot()
		: recipient_(0),
		  method_(0)
	{}

	Slot(Recipient *recipient, Method method)
		: recipient_(recipient),
		  method_(method)
	{}

	void invoke(Value value) { (recipient_->*method_)(value); }

private:
	Recipient *recipient_;
	Method method_;
};

template<class Value>
class Signal: public Object
{
public:
	inline static Ref<Signal> create() { return new Signal; }

	void emit(Value value) {
		for (int i = 0; i < callbacks()->size(); ++i) {
			CallbackList *list = callbacks()->valueAt(i);
			for (int j = 0; j < list->size(); ++j)
				list->at(j)->invoke(value);
		}
	}

	template<class Recipient>
	void connect(Recipient *recipient, void (Recipient::* method)(Value)) {
		Ref<CallbackList> list;
		if (!callbacks()->lookup(recipient, &list)) {
			list = CallbackList::create();
			callbacks()->insert(recipient, list);
		}
		list->append(new Slot<Recipient, Value>(recipient, method));
	}

	void disconnect(void *recipient) {
		callbacks()->remove(recipient);
	}

private:
	friend class Connection;
	typedef List< Ref< Callback<Value> > > CallbackList;
	typedef Map<void*, Ref<CallbackList> > CallbackListByRecipient;

	Signal() {}

	inline CallbackListByRecipient *callbacks() {
		if (!callbacks_) callbacks_ = CallbackListByRecipient::create();
		return callbacks_;
	}

	Ref<CallbackListByRecipient> callbacks_;
};

template<class T>
class Property
{
public:
	Property() {}
	Property(const T &b): value_(b) {}

	inline operator const T&() const { return value_; }
	inline Property &operator=(const T &b) {
		if (b != value_) {
			value_ = b;
			if (valueChanged_)
				valueChanged_->emit(value_);
		}
		return *this;
	}

	Signal<T> *valueChanged() {
		if (!valueChanged_) valueChanged_ = Signal<T>::create();
		return valueChanged_;
	}

	inline String toString() const { return Format() << value_; }
	inline operator String() const { return toString(); }

	Property *operator->() { return this; }
	const Property *operator->() const { return this; }

private:
	Ref< Signal<T> > valueChanged_;
	T value_;
};

} // namespace fkit

#endif // FKIT_PROPERTY_H
