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

		explicit entity(entity_index_t ref)
			: entity_ref_(ref)
		{}

		entity_index_t index() const
		{
			return entity_ref_;
		}

		bool operator==(entity const& rhs) const
		{
			return entity_ref_ == rhs.entity_ref_; 
		}

		bool operator<(entity const& rhs) const
		{
			return entity_ref_ < rhs.entity_ref_;
		}

	private:

		entity_index_t entity_ref_;
	};

	inline entity make_entity(entity_index_t idx)
	{
		return entity(idx);
	}
}

#endif // _ENTITY_ENTITY_H_INCLUDED_
