/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EVENTQUEUE_THUNK_H_
#define EVENTQUEUE_THUNK_H_

#include "AlignedStorage.h"
#include "detail/ThunkVTable.h"

namespace eq {

// forward declaration of ThunkVTableGenerator
namespace detail {
template<typename T, std::size_t BufferSize>
class ThunkVTableGenerator;
}

/**
 * A Thunk is a container holding any kind of nullary callable.
 * It wrap value semantic and function call operations of the inner callable
 * held.
 * \note Thunk of callable bound to arguments should be generated by the
 * function make_thunk.
 */
template<std::size_t BufferSize>
class Thunk {
	template<typename T>
	friend class detail::ThunkVTableGenerator;

public:

	/**
	 * Thunk Empty constructor.
	 * When this thunk is called, if does nothing.
	 */
	Thunk();

	/**
	 * Construct a Thunk from a nullary callable of type F.
	 * When the call operator is invoked, it call a copy of f ( f() ).
	 */
	template<typename F>
	Thunk(const F& f);

	/**
	 * Special constructor for pointer to function.
	 * Allow references to functions to gracefully decay into pointer to function.
	 * Otherwise, reference to function are not copy constructible (their is no
	 * constructible function type in C++).
	 * When the call operator is invoked, it call a copy of f ( f() ).
	 */
	Thunk(void (*f)());

	/**
	 * Copy construction of a thunk.
	 * Take care that the inner F is correctly copied.
	 */
	Thunk(const Thunk& other) : _storage(), _vtable() {
		other._vtable->copy(*this, other);
	}

	/**
	 * Destruction of the Thunk correctly call the destructor of the
	 * inner callable.
	 */
	~Thunk() {
		_vtable->destroy(*this);
	}

	/**
	 * Copy assignement from another thunk.
	 * Ensure that the callable held is correctly destroyed then copy
	 * the correctly copy the new one.
	 */
	Thunk& operator=(const Thunk& other) {
		if (this == &other) {
			return *this;
		}
		_vtable->destroy(*this);
		other._vtable->copy(*this, other);
		return *this;
	}

	/**
	 * Call operator. Invoke the inner callable.
	 */
	void operator()() const {
		_vtable->call(*this);
	}

private:
	static void empty_thunk() { }

	AlignedStorage<char[BufferSize]> _storage;
	const detail::ThunkVTable<BufferSize>* _vtable;
};

} // namespace eq

#include "detail/Thunk.impl.h"

#endif  /* EVENTQUEUE_THUNK_H_ */
