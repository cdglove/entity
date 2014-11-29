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
#include <boost/pool/pool.hpp>
#include <daily/timer/instrument.h>

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
			: entity_pool_(sizeof(entity_index_t))
		{}

		~entity_pool()
		{
			while(!entities_.empty())
			{
				destroy(make_entity(*entities_.back()));
			}
		}

		entity_handle create()
		{
			entity_index_t* new_idx = new(entity_pool_.malloc()) entity_index_t();
			*new_idx = entities_.size();
			entity_handle ret_handle(new_idx);
			entities_.push_back(new_idx);
			signals().on_entity_create(ret_handle.get());
			return ret_handle;
		}	

		void destroy(entity e)
		{
			// Avoid swapping if this is at the end.
			if(e.index() + 1 < entities_.size())
			{
				swap_entities(e.index(), *entities_.back());
			}

			entity_index_t* idx = entities_.back();
			entities_.pop_back();
			signals().on_entity_destroy(make_entity(*idx));
			idx->~entity_index_t();
			entity_pool_.free(idx);
		}

		entity_handle handle(entity e)
		{
			return entity_handle(entities_[e.index()]);
		}

		std::size_t size() const
		{
			return entities_.size();
		}

		iterator begin() const
		{
			return iterator_impl(0);
		}

		iterator end() const
		{
			return iterator_impl(size());
		}

		signal_list& signals()
		{
			return signals_;
		}

	private:

		entity_pool(entity_pool const&);
		entity_pool operator=(entity_pool);

		void swap_entities(entity_index_t a, entity_index_t b)
		{
			using std::swap;
			swap(entities_[a], entities_[b]);
			swap(*entities_[a], *entities_[b]);
			signals().on_entity_swap(make_entity(a), make_entity(b));
		}

		boost::pool<> entity_pool_;
		std::vector<entity_index_t*> entities_;
		signal_list signals_;
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
