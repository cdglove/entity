//! \file entity/detail/entity_allocator.h
//
// Allocator for entities, so we're not heap allocating
// every little int.
// 
#pragma once
#ifndef _ENTITY_DETAIL_ENTITYALLOCATOR_H_INCLUDED_
#define _ENTITY_DETAIL_ENTITYALLOCATOR_H_INCLUDED_

#include "entity/entity.h"
#include "entity/entity_pool.h"
#include <boost/pool/pool.hpp>
#include <boost/assert.hpp>
#include <type_traits>

// ----------------------------------------------------------------------------
//
namespace entity { namespace detail {

template<typename T>
class entity_allocator
{
public:

	typedef T value_type;

	entity_allocator(entity_pool& owner_pool)
		: entity_pool_(owner_pool)
	{}

	template <typename U> 
	entity_allocator(entity_allocator<U> const& other)
		: entity_pool_(other.entity_pool_)
	{}

	T* allocate(std::size_t n)
	{
		BOOST_ASSERT(n == 1);
		BOOST_ASSERT(sizeof(T) <= entity_pool_.entity_pool_.get_requested_size());
		return static_cast<T*>(entity_pool_.entity_pool_.malloc());
	}

	void deallocate(T* p, std::size_t n)
	{
		BOOST_ASSERT(n == 1);
		entity_pool_.entity_pool_.free(p);
	}

	void destroy(T* p)
	{
		destroy_impl(p);
	}

	// allocate_shared on MSVC 18 isn't using allocator traits
#if _MSC_VER < 1900
	template<typename U>
	struct rebind 
	{ 
		typedef entity_allocator<U> other;
	};
#endif

private:

	template<typename U>
	typename std::enable_if<
		!std::is_same<
			U, entity_index_t
		>::value
	>::type destroy_impl(U* p)
	{
		p->~T();
	}

	template<typename U>
	typename std::enable_if<
		std::is_same<
		U, entity_index_t
		>::value
	>::type destroy_impl(U* p)
	{
		entity_pool_.destroy_impl(*p);
	}

	template <typename U>
	friend class entity_allocator;

	template <typename U, typename V>
	friend bool operator==(entity_allocator<U> const& a, entity_allocator<U> const& b);

	template <typename U, typename V>
	friend bool operator!=(entity_allocator<V> const& a, entity_allocator<V> const& b);

	entity_pool& entity_pool_;
};

// ----------------------------------------------------------------------------
//
template <typename U, typename V>
bool operator==(entity_allocator<U> const& a, entity_allocator<V> const& b)
{
	return &a.entity_pool_ == &b.entity_pool_;
}

template <typename U, typename V>
bool operator!=(entity_allocator<U> const& a, entity_allocator<V> const& b)
{
	return &a.entity_pool_ != &b.entity_pool_;
}

}}

#endif // _ENTITY_DETAIL_ENTITYALLOCATOR_H_INCLUDED_