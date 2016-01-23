// ****************************************************************************
// entity/entity.hpp
//
// Represents an entity which is made up of components.
// Essentially an ID but not incrementable, etc.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_ENTITY_H_INCLUDED_
#define _ENTITY_ENTITY_H_INCLUDED_

#include <boost/operators.hpp>
#include <algorithm>
#include <functional>
#include <memory>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity_index.hpp"

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	class entity : boost::totally_ordered<entity>
	{
	public:

		entity_index_t index() const
		{
			return idx_;
		}

		bool operator==(entity const& rhs) const
		{
			return index() == rhs.index(); 
		}

		bool operator<(entity const& rhs) const
		{
			return index() < rhs.index();
		}

	private:

		friend entity make_entity(entity_index_t) BOOST_NOEXCEPT_OR_NOTHROW;

		// Only make_entity can construct entities.
		// Can consider impicit conversion here, but
		// perfer to be conservative at first.
		explicit entity(entity_index_t idx)
			: idx_(idx)
		{}

		entity_index_t idx_;
	};

	inline entity make_entity(entity_index_t idx) BOOST_NOEXCEPT_OR_NOTHROW
	{
		return entity(idx);
	}

	// ------------------------------------------------------------------------
	//
	class unique_entity : boost::totally_ordered<unique_entity>
	{
	public:

		unique_entity()
		{}

		entity get() const
		{
			return make_entity(*ref_);
		}

		bool operator==(unique_entity const& rhs) const
		{
			return get() == rhs.get(); 
		}

		bool operator<(unique_entity const& rhs) const
		{
			return get() < rhs.get();
		}

		void clear()
		{
			ref_ = nullptr;
		}

	private:

		friend class entity_pool;
		friend class shared_entity;
		friend void swap(unique_entity&, unique_entity&);
		
		typedef std::unique_ptr<
			const entity_index_t, 
			std::function<void(entity_index_t const*)>
		> const_ref_type;

		typedef std::unique_ptr<
			entity_index_t, 
			std::function<void(entity_index_t const*)>
		> ref_type;
		
		unique_entity(const_ref_type ref)
			: ref_(std::move(ref))
		{}

		unique_entity(ref_type ref)
			: ref_(std::move(ref))
		{}

		const_ref_type ref_;
	};

	inline void swap(unique_entity& a, unique_entity& b)
	{
		std::swap(a.ref_, b.ref_);
	}

	class shared_entity : boost::totally_ordered<shared_entity>
	{
	public:

		shared_entity()
		{}

		shared_entity(unique_entity&& ref)
			: ref_(std::move(ref.ref_))
		{}

		entity get() const
		{
			return make_entity(*ref_);
		}

		bool operator==(shared_entity const& rhs) const
		{
			return get() == rhs.get(); 
		}

		bool operator<(shared_entity const& rhs) const
		{
			return get() < rhs.get();
		}

		void clear()
		{
			ref_ = nullptr;
		}

	private:

		friend class entity_pool;
		friend class weak_entity;
		friend void swap(shared_entity&, shared_entity&);

		typedef std::shared_ptr<const entity_index_t> const_ref_type;
		typedef std::shared_ptr<entity_index_t> ref_type;

		shared_entity(const_ref_type ref)
			: ref_(std::move(ref))
		{}

		shared_entity(ref_type ref)
			: ref_(std::move(ref))
		{}

		const_ref_type ref_;
	};

	inline void swap(shared_entity& a, shared_entity& b)
	{
		std::swap(a.ref_, b.ref_);
	}

	// ------------------------------------------------------------------------
	//
	class weak_entity : boost::totally_ordered<weak_entity>
	{
	public:

		weak_entity(shared_entity const& ref)
			: ref_(ref.ref_)
		{}

		shared_entity lock() const
		{
			return ref_.lock();
		}

		bool operator==(weak_entity const& rhs) const
		{
			return ref_.lock() == rhs.ref_.lock();
		}

		bool operator<(weak_entity const& rhs) const
		{
			return ref_.lock() < rhs.ref_.lock();
		}

	private:

		friend class shared_entity;
		std::weak_ptr<const entity_index_t> ref_;
	};
}

#endif // _ENTITY_ENTITY_H_INCLUDED_
