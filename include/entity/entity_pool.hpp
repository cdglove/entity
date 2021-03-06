// ****************************************************************************
// entity/entity_pool.h
//
// Represents a pool of entity ids.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef ENTITY_ENTITYPOOL_H_INCLUDED_
#define ENTITY_ENTITYPOOL_H_INCLUDED_

#include <boost/function.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/pool/pool.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/optional_last_value.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <cstddef>
#include <new>
#include <type_traits>
#include <vector>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"
#include "entity/entity_index.hpp"

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
			: entity_pool_(16)
		{}

		~entity_pool()
		{
			while(!entities_.empty())
			{
				destroy(make_entity(*entities_.back()));
			}
		}

		entity create()
		{
			entity_index_t* new_idx = new(entity_pool_.malloc()) entity_index_t(entities_.size());
			entity ret_val = make_entity(*new_idx);
			entities_.push_back(new_idx);
			signals().on_entity_create(ret_val);
			return ret_val;
		}	

		unique_entity create_unique()
		{
			entity_index_t* new_idx_ptr = nullptr;
			void* new_index_mem = nullptr;
			bool pop_on_catch = false;
			BOOST_TRY
			{	
				new_index_mem = entity_pool_.malloc();
				new_idx_ptr = new(new_index_mem) entity_index_t(entities_.size());

				entity ent_val = make_entity(*new_idx_ptr);
				entities_.push_back(new_idx_ptr);

				// Ensure we leave the container in a good state if shared_ptr throws.
				pop_on_catch = true;

				unique_entity::ref_type new_idx(
					new_idx_ptr,
					entity_deleter(*this)
				);

				signals().on_entity_create(ent_val);
				return std::move(new_idx);
			}
			BOOST_CATCH(...)
			{
				if(pop_on_catch)
					entities_.pop_back();
				if(new_idx_ptr)
					new_idx_ptr->~entity_index_t();
				if(new_index_mem)
					entity_pool_.free(new_index_mem);
				BOOST_RETHROW;
			}
			BOOST_CATCH_END
		}

		shared_entity create_shared()
		{
			return create_unique();
		}

		void destroy(entity e)
		{
			destroy_impl(e.index());
		}

		std::size_t size() const
		{
			return entities_.size();
		}

		bool empty() const
		{
			return entities_.empty();
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

		struct entity_deleter
		{
			entity_deleter(entity_pool& owner_pool)
				: owner_pool_(owner_pool)
			{}

			void operator()(entity_index_t const* p)
			{
				owner_pool_.destroy_impl(*p);
			}

			entity_pool& owner_pool_;
		};

		void swap_entities(entity_index_t a, entity_index_t b)
		{
			using std::swap;
			swap(entities_[a], entities_[b]);
			swap(*entities_[a], *entities_[b]);
			signals().on_entity_swap(make_entity(a), make_entity(b));
		}

		void destroy_impl(entity_index_t e)
		{
			// Avoid swapping if this is at the end.
			if((e + 1) < entities_.size())
			{
				swap_entities(e, *entities_.back());
			}

			entity_index_t* idx = entities_.back();
			entities_.pop_back();
			BOOST_TRY
			{
				signals().on_entity_destroy(make_entity(*idx));
				idx->~entity_index_t();
				void* idx_mem = idx;
				idx = nullptr;
				entity_pool_.free(idx_mem);
			}
			BOOST_CATCH(...)
			{
				if(idx)
					entities_.push_back(idx);
				BOOST_RETHROW;
			}
			BOOST_CATCH_END
		}

		boost::pool<> entity_pool_;
		std::vector<entity_index_t*> entities_;
		signal_list signals_;
	};
}

#endif // ENTITY_ENTITYPOOL_H_INCLUDED_
