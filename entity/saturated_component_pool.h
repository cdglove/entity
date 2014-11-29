//! \file entity/saturated_component_pool.h
//
// Represents a component pool where the number of components
// equals the number of entities.  Allows assumptions to be
// made to optimize bookkeeping and iteration.
// 
#pragma once
#ifndef _ENTITY_SATURATEDCOMPONENTPOOL_H_INCLUDED_
#define _ENTITY_SATURATEDCOMPONENTPOOL_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_pool.h"
#include "entity/entity_component_iterator.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/assert.hpp>
#include <cstddef>
#include <memory>
#include <vector>
#include <daily/timer/instrument.h>

// ----------------------------------------------------------------------------
//
namespace entity 
{
	template<typename ComponentPool>
	class component_pool_creation_queue;

	template<typename ComponentPool>
	class component_pool_destruction_queue;

	template<typename T>
	class saturated_component_pool
	{
	private:

		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			iterator_impl()
			{}

			entity get_entity() const
			{
				return make_entity(entity_index_);
			}

			void advance_to_target_entity(entity_index_t e)
			{
				entity_index_ = e;
			}

			T* maybe_extract_ptr(entity ent) const
			{
				return parent_->get_component(ent);
			}
			
			bool is_valid() const
			{
				return true;
			}

		private:

			friend class boost::iterator_core_access;
			friend class saturated_component_pool;
			
			typedef T* parent_iterator;

			iterator_impl(
				saturated_component_pool* parent,
				entity_index_t idx)
				: parent_(parent)
				, entity_index_(idx)
			{}

			void increment()
			{
				DAILY_AUTO_INSTRUMENT_NODE(saturated_component_pool_iterator__increment);
				++entity_index_;
			}

			bool equal(iterator_impl const& other) const
			{
				return entity_index_ == other.entity_index_;
			}

			T& dereference() const
			{
				return parent_->components_[entity_index_];
			}

			saturated_component_pool* parent_;
			entity_index_t entity_index_;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;
		
		saturated_component_pool(entity_pool& owner_pool, T const& default_value = T())
		{
			// Create default values for existing entities.
			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				boost::bind(
					&saturated_component_pool::create<T const&>,
					this,
					::_1,
					boost::ref(default_value)
				)
			);

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&saturated_component_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&saturated_component_pool::handle_swap_entity,
						this,
						::_1, 
						::_2
					)
				)
			;
		}

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		void auto_create_components(entity_pool& owner_pool, Args... constructor_args)
		{
			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					std::function<void(entity)>(
						[this, constructor_args...](entity e) 
						{
							create(e, constructor_args...);
						}
					)
				)
			;
		}
	#else
		void auto_create_components(entity_pool& owner_pool, T const& default_value)
		{
			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					boost::bind(
						&saturated_component_pool::create,
						this,
						::_1,
						default_value
					)
				)
			;
		}
	#endif

		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			DAILY_AUTO_INSTRUMENT_NODE(saturated_component_pool__create);
			components_.emplace(components_.begin() + e.index(), std::forward<Args>(args)...);
			return &components_[e.index()];
		}	

		void destroy(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(saturated_component_pool__destroy);
			components_.erase(components_.begin() + e.index());
		}

		T* get(entity e)
		{
			return get_component(e);
		}

		T const* get(entity e) const
		{
			return get_component(e);
		}

		iterator begin()
		{
			return iterator(this, 0);
		}

		iterator end()
		{
			return iterator(this, components_.size());
		}

		std::size_t size()
		{
			return components_.size();
		}

	private:

		friend class component_pool_creation_queue<saturated_component_pool<type>>;
		friend class component_pool_destruction_queue<saturated_component_pool<type>>;

		struct slot_list
		{
			boost::signals2::scoped_connection entity_create_handler;
			boost::signals2::scoped_connection entity_destroy_handler;
			boost::signals2::scoped_connection entity_swap_handler;
		};

		T* get_component(entity e)
		{
			return &components_[e.index()];
		}

		T const* get_component(entity e) const
		{
			return &components_[e.index()];
		}

		// --------------------------------------------------------------------
		// Queue interface.
		template<typename Iter>
		void create_range(Iter first, Iter last)
		{
			while(first != last)
			{
				components_.emplace_back(std::move(first->second));
				++first;
			}
		}

		template<typename Iter>
		void destroy_range(Iter current, Iter last)
		{
			while(current != last)
			{
				destroy(current->get());
				++current;
			}
		}

		// --------------------------------------------------------------------
		// Slot Handlers.
		void handle_destroy_entity(entity e)
		{
			destroy(e);
		}

		void handle_swap_entity(entity a, entity b)
		{
			using std::swap;
			swap(components_[a.index()], components_[b.index()]);
		}

		std::vector<type> components_;
		slot_list		  slots_;
	};
}

#endif // _ENTITY_SATURATEDCOMPONENTPOOL_H_INCLUDED_
