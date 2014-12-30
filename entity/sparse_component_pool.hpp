//! \file entity/sparse_component_pool.h
//
// Represents a component pool where the number of components
// is significantly less than the number of entities with those components.
// Uses less space and will be faster to iterate under these conditions.
// 
#pragma once
#ifndef _ENTITY_SPARSECOMPONENT_POOL_H_INCLUDED_
#define _ENTITY_SPARSECOMPONENT_POOL_H_INCLUDED_

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity_pool.hpp"
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/container/flat_map.hpp>
#include <cstddef>
#include <daily/timer/instrument.h>

// ----------------------------------------------------------------------------
//
namespace entity 
{
	template<typename ComponentPool>
	class component_creation_queue;

	template<typename ComponentPool>
	class component_destruction_queue;

	template<typename T>
	class sparse_component_pool
	{
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			entity get_entity() const
			{
				return iterator_->first;
			}

			iterator_impl()
			{}

		private:

			friend class boost::iterator_core_access;
			friend class sparse_component_pool;
			
			typedef typename boost::container::flat_map<entity, T>::iterator parent_iterator;

			explicit iterator_impl(parent_iterator convert_from)
				: iterator_(std::move(convert_from))
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
				return iterator_->second;
			}

			parent_iterator iterator_;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;

		// --------------------------------------------------------------------
		//
		struct window
		{
			typedef type value_type;

			window()
			{}

			bool is_entity(entity e) const
			{
				return iterator_ != end_ && e == iterator_->first;
			}

			bool increment(entity target)
			{
				while(iterator_ != end_ && iterator_->first < target)
					++iterator_;

				return iterator_ != end_ && iterator_->first == target;
			}

			bool advance(entity target)
			{
				return increment(target);
			}

			value_type& get() const
			{
				return iterator_->second;
			}

		private:

			friend class sparse_component_pool;

			typedef typename boost::container::flat_map<
				entity, value_type
			>::iterator parent_iterator;

			window(parent_iterator start, parent_iterator end)
				: iterator_(std::move(start))
				, end_(std::move(end))
			{}

			parent_iterator iterator_;
			parent_iterator end_;
		};
		
		// --------------------------------------------------------------------
		//
		sparse_component_pool(entity_pool& owner_pool, T const& default_value = T())
		{
			// Create default values for existing entities.
			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				boost::bind(
					&sparse_component_pool::create<T const&>,
					this,
					::_1,
					boost::ref(default_value)
				)
			);

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&sparse_component_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&sparse_component_pool::handle_swap_entity,
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
						&sparse_component_pool::create,
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
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__create);
			auto r = components_.emplace(e, std::forward<Args>(args)...);
			return &(r.first->second);
		}
	#else
		T* create(entity e, type&& original)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__create);
			auto r = components_.emplace(e, std::move(original));
			return &(r.first->second);
		}
	#endif

		void destroy(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__destroy);
			components_.erase(e);
		}

		T* get(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__get);
			auto obj = components_.find(e);
			if(obj != components_.end())
			{
				return &obj->second;
			}

			return nullptr;
		}

		T const* get(entity e) const
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__get);
			auto obj = components_.find(e);
			if(obj != components_.end())
			{
				return &obj->second;
			}

			return nullptr;
		}

		iterator begin()
		{
			return iterator(components_.begin());
		}

		iterator end()
		{
			return iterator(components_.end());
		}

		window view()
		{
			return window(components_.begin(), components_.end());
		}

		std::size_t size()
		{
			return components_.size();
		}

	private:

		friend class component_creation_queue<sparse_component_pool<type>>;
		friend class component_destruction_queue<sparse_component_pool<type>>;

		struct slot_list
		{
			boost::signals2::scoped_connection entity_create_handler;
			boost::signals2::scoped_connection entity_destroy_handler;
			boost::signals2::scoped_connection entity_swap_handler;
		};

		// --------------------------------------------------------------------
		// Queue interface.
		template<typename Iter>
		void create_range(Iter first, Iter last)
		{
			std::vector<std::pair<entity, T>> entities;
			std::transform(first, last, std::back_inserter(entities), [entities](std::pair<weak_entity, type>& h)
			{
				return std::make_pair(h.first.lock().get(), std::move(h.second));
			});

			components_.insert(boost::container::ordered_unique_range_t(), entities.begin(), entities.end());
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
		// Slot Handlers.
		void handle_destroy_entity(entity e)
		{
			destroy(e);
		}

		void handle_swap_entity(entity a, entity b)
		{
			using std::swap;
			auto c_a = components_.find(a);
			auto c_b = components_.find(b);
			auto end = components_.end();

			if(c_a != end && c_b != end)
			{
				swap(c_a->second, c_b->second);
			}
			else if(c_a != end)
			{
				components_.emplace(b, std::move(c_a->second));
				components_.erase(a);
			}
			else if(c_b != end)
			{
				components_.emplace(a, std::move(c_b->second));
				components_.erase(b);
			}
		}

		boost::container::flat_map<entity, T> components_;
		slot_list slots_;
	};
}

#endif // _ENTITY_DENSECOMPONENT_POOL_H_INCLUDED_
