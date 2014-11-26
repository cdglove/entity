//! \file entity/entity_pool.h
//
// Represents a pool of entity ids.
// 
#pragma once
#ifndef _ENTITY_ENTITYPOOL_H_INCLUDED_
#define _ENTITY_ENTITYPOOL_H_INCLUDED_

#include "entity/entity.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

// ----------------------------------------------------------------------------
//
namespace entity
{
	class entity_pool
	{
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , entity
			  , boost::forward_traversal_tag
			  , entity
		  	>
		{
		private:

			friend class boost::iterator_core_access;
			friend class entity_pool;
			
			iterator_impl(entity_index_t idx)
				: iterator_(idx)
			{}

			void increment()
			{
				++iterator_;
			}

			bool equal(iterator_impl const& other) const
			{
				return iterator_ == other.iterator_;
			}

			entity dereference() const
			{
				return make_entity(iterator_);
			}

			entity_index_t iterator_;
		};

	public:

		typedef iterator_impl iterator;
		typedef iterator_impl const_iterator;

		entity_pool(entity_index_t max_entities)
			: max_entities_(max_entities)
			, num_entities_(0)
		{}

		entity create()
		{
			return make_entity(num_entities_++);
		}	

		void destroy(entity e)
		{
			--num_entities_;
		}

		std::size_t size() const
		{
			return num_entities_;
		}

		iterator begin() const
		{
			return iterator_impl(0);
		}

		iterator end() const
		{
			return iterator_impl(num_entities_);
		}

	private:

		entity_pool(entity_pool const&);
		entity_pool operator=(entity_pool);

		entity_index_t max_entities_;
		entity_index_t num_entities_;
	};

	entity_pool::iterator begin(entity_pool const& p)
	{
		return p.begin();
	}

	entity_pool::iterator end(entity_pool const& p)
	{
		return p.end();
	}

}

#endif // _ENTITY_ENTITYPOOL_H_INCLUDED_
