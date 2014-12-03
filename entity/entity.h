//! \file entity/entity.h
//
// Represents an entity which is made up of components.
// Essentially an ID but not incrementable, etc.
// 
#pragma once
#ifndef _ENTITY_ENTITY_H_INCLUDED_
#define _ENTITY_ENTITY_H_INCLUDED_

#include "entity/config.h"
#include <boost/operators.hpp>
#include <memory>

// ----------------------------------------------------------------------------
//
namespace entity
{
	typedef std::size_t entity_index_t;

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

		friend entity make_entity(entity_index_t) noexcept;

		// Only make entity can construct entities.
		// Can consider impicit conversion here, but
		// perfer to be conservative at first.
		explicit entity(entity_index_t idx)
			: idx_(idx)
		{}

		entity_index_t idx_;
	};

	inline entity make_entity(entity_index_t idx) noexcept
	{
		return entity(idx);
	}

	// ------------------------------------------------------------------------
	//
	class unique_entity : boost::totally_ordered<unique_entity>
	{

	};

	// ------------------------------------------------------------------------
	//
	class weak_entity;
	class shared_entity;
	
	class shared_entity : boost::totally_ordered<shared_entity>
	{
	public:

		shared_entity()
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

		shared_entity(std::shared_ptr<const entity_index_t> ref)
			: ref_(std::move(ref))
		{}

		shared_entity(std::shared_ptr<entity_index_t> ref)
			: ref_(std::move(ref))
		{}

		std::shared_ptr<const entity_index_t> ref_;
	};

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
