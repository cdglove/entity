// ****************************************************************************
// entity/support/object_storage.hpp
//
// Fixed size storage that can be used for storing polymorphic object in a 
// homogenous container. 
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef ENTITY_SUPPORT_OBJECTSTORAGE_H_INCLUDED_
#define ENTITY_SUPPORT_OBJECTSTORAGE_H_INCLUDED_

#include <type_traits>

// ----------------------------------------------------------------------------
//
namespace entity { namespace support {

template<typename Base, std::size_t Size, std::size_t Alignment>
class object_storage
{
	typename std::aligned_storage<Size, Alignment>::type storage_;

public:

	template<typename T>
	object_storage(T&& t)
	{
		static_assert(sizeof(T) <= Size, "T won't fit in current size.");
		// Ensure we are of compatible types.
		Base* b = new(&storage_) T(std::move(t));
		(void)b;
	}

	~object_storage()
	{
		reinterpret_cast<Base*>(&storage_)->~Base();
	}

	Base* operator->()
	{
		return reinterpret_cast<Base*>(&storage_);
	}

	Base& operator*()
	{
		return *reinterpret_cast<Base*>(&storage_);
	}

	Base const* operator->() const
	{
		return reinterpret_cast<Base*>(&storage_);
	}

	Base const& operator*()
	{
		return *reinterpret_cast<Base*>(&storage_);
	}
};

} } // namespace entity { namespace support {

#endif // ENTITY_SUPPORT_OBJECTSTORAGE_H_INCLUDED_