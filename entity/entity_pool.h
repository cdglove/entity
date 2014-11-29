//! \file entity/entity_pool.h
//
// Represents a pool of entity ids.
// 
#pragma once
#ifndef _ENTITY_ENTITYPOOL_H_INCLUDED_
#define _ENTITY_ENTITYPOOL_H_INCLUDED_

#include "entity/entity.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/signals2/signal.hpp>
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

		struct signal_list
		{
			boost::signals2::signal<void(entity)> on_entity_create;
			boost::signals2::signal<void(entity)> on_entity_destroy;
			boost::signals2::signal<void(entity, entity)> on_entity_swap;
		};

		entity_pool()
			: num_entities_(0)
		{}

		entity create()
		{
			entity new_entity = make_entity(num_entities_++);
			signals().on_entity_create(new_entity);
			return new_entity;
		}	

		void destroy(entity e)
		{
			--num_entities_;
			signals().on_entity_swap(e, make_entity(num_entities_));
			signals().on_entity_destroy(make_entity(num_entities_));
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

		signal_list& signals()
		{
			return signals_;
		}

	private:

		entity_pool(entity_pool const&);
		entity_pool operator=(entity_pool);

		entity_index_t num_entities_;
		signal_list    signals_;
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
