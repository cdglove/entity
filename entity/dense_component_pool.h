//! \file entity/dense_component_pool.h
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
#pragma once
#ifndef _ENTITY_DENSECOMPONENTPOOL_H_INCLUDED_
#define _ENTITY_DENSECOMPONENTPOOL_H_INCLUDED_

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
	class dense_component_pool
	{
	private:

		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			entity get_entity() const
			{
				return make_entity(std::distance(begin_, iterator_));
			}

			iterator_impl()
			{}

			void advance_to_target_entity(entity_index_t target)
			{
				iterator_ = begin_ + target;
			}

			T* maybe_extract_ptr(entity ent) const
			{
				if(!parent_->is_available(ent.index()))
				{
					return parent_->get_component(ent);
				}

				return nullptr;
			}

			bool is_valid() const
			{
				return !parent_->is_available(get_entity().index());
			}

		private:

			friend class boost::iterator_core_access;
			friend class dense_component_pool;
			
			typedef typename std::vector<char>::iterator parent_iterator;

			iterator_impl(dense_component_pool* parent, parent_iterator start, parent_iterator first)
				: parent_(parent)
				, iterator_(std::move(start))
				, begin_(std::move(first))
			{}

			void increment()
			{
				++iterator_;
			}

			bool equal(iterator_impl const& other) const
			{
				return iterator_ == other.iterator_;
			}

			T& dereference() const
			{
				return *parent_->get_component(get_entity());
			}

			dense_component_pool* parent_;
			parent_iterator iterator_;
			parent_iterator begin_;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;
		
		dense_component_pool(entity_pool& owner_pool, T const& default_value = T())
			: used_count_(0)
		{
			// Create default values for existing entities.
			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				boost::bind(
					&dense_component_pool::create<T const&>,
					this,
					::_1,
					boost::ref(default_value)
				)
			);

			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					boost::bind(
						&dense_component_pool::handle_create_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&dense_component_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&dense_component_pool::handle_swap_entity,
						this,
						::_1,
						::_2
					)
				)
			;
		}

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		void auto_create_components(entity_pool& owner_pool, Args&&... constructor_args)
		{
			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					std::function<void(entity)>(
						[this, constructor_args...](entity e)
						{
							create_entity_slot(e);
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
						&dense_component_pool::create,
						this,
						::_1,
						default_value
					)
				)
			;
		}
	#endif

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__create);
			set_available(e.index(), false);
			T* ret_val = get_component(e);
			new(ret_val) T(std::forward<Args>(args)...);
			++used_count_;
			return ret_val;
		}	
	#else
		T* create(entity e, type&& original)
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__create);
			set_available(e.index(), false);
			T* ret_val = get_component(e);
			new(ret_val) T(std::move(original));
			return ret_val;
		}	
	#endif

		void destroy(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__destroy);

			BOOST_ASSERT(!is_available(e.index()) && "Trying to destroy un-allocated component.");
			--used_count_;
			T* p = get_component(e);
			p->~T();
			
			set_available(e.index(), true);
		}

		T* get(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__get);
			if(is_available(e.index()))
			{
				return nullptr;
			}

			return get_component(e);
		}

		T const* get(entity e) const
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__get);
			if(is_available(e.index()))
			{
				return nullptr;
			}

			return get_component(e);
		}

		iterator begin()
		{
			return iterator(this, available_.begin(), available_.begin());
		}

		iterator end()
		{
			return iterator(this, available_.end(), available_.begin());
		}

		std::size_t size()
		{
			return used_count_;
		}

	private:

		friend class component_pool_creation_queue<dense_component_pool<type>>;
		friend class component_pool_destruction_queue<dense_component_pool<type>>;

		struct slot_list
		{
			boost::signals2::scoped_connection entity_create_handler;
			boost::signals2::scoped_connection entity_destroy_handler;
			boost::signals2::scoped_connection entity_swap_handler;
		};

		T* get_component(entity e)
		{
			return reinterpret_cast<T*>(&components_[e.index()]);
		}

		T const* get_component(entity e) const
		{
			return reinterpret_cast<T*>(&components_[e.index()]);
		}

		bool is_available(entity_index_t idx)
		{
			return available_[idx] != 0;
		}

		void set_available(entity_index_t idx, bool available)
		{
			available_[idx] = (available == true);
		}

		void create_entity_slot(entity e)
		{
			components_.insert(components_.begin() + e.index(), element_t());
			available_.emplace(available_.begin() + e.index(), true);
		}

		void free_entity_slot(entity e)
		{
			available_.erase(available_.begin() + e.index());
			components_.erase(components_.begin() + e.index());
		}

		// --------------------------------------------------------------------
		// Queue interface.
		template<typename Iter>
		void create_range(Iter current, Iter last)
		{
			while(current != last)
			{
				create(current->first.lock().get(), std::move(current->second));
				++current;
			}
		}

		template<typename Iter>
		void destroy_range(Iter current, Iter last)
		{
			while(current != last)
			{
				destroy(current->lock().get());
				++current;
			}
		}

		// --------------------------------------------------------------------
		// Slot Handlers
		void handle_create_entity(entity e)
		{
			create_entity_slot(e);
		}

		void handle_destroy_entity(entity e)
		{
			if(!is_available(e.index()))
			{
				destroy(e);
			}

			free_entity_slot(e);
		}

		void handle_swap_entity(entity a, entity b)
		{
			using std::swap;
			swap(components_[a.index()], components_[b.index()]);
		}

		struct element_t
		{
			char mem_[sizeof(T)];
		};

		std::vector<element_t>			components_;
		std::vector<char>				available_;
		std::size_t						used_count_;
		slot_list						slots_;
	};
}
	
#endif // _ENTITY_DENSECOMPONENTPOOL_H_INCLUDED_
