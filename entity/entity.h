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

		friend entity make_entity(entity_index_t);

		// Only make entity can construct entities.
		// Can consider impicit conversion here, but
		// perfer to be conservative at first.
		explicit entity(entity_index_t idx)
			: idx_(idx)
		{}

		entity_index_t idx_;
	};

	inline entity make_entity(entity_index_t idx)
	{
		return entity(idx);
	}

	// ------------------------------------------------------------------------
	//
	class entity_handle : boost::totally_ordered<entity_handle>
	{
	public:

		entity get() const
		{
			return make_entity(*ref_);
		}

		bool operator==(entity_handle const& rhs) const
		{
			return get() == rhs.get(); 
		}

		bool operator<(entity_handle const& rhs) const
		{
			return get() < rhs.get();
		}

	private:

		friend class entity_pool;

		// Only entity_pool can create entity handles.
		explicit entity_handle(entity_index_t const* ref)
			: ref_(ref)
		{}

		entity_index_t const* ref_;
	};
}

#endif // _ENTITY_ENTITY_H_INCLUDED_
