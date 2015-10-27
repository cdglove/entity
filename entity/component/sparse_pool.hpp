// ****************************************************************************
// entity/component/sparse_pool.h
//
// Represents a component pool where the number of components
// is significantly less than the number of entities with those components.
// Uses less space and will be faster to iterate under these conditions.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_
#define _ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_

#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/container/container_fwd.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/ref.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <mutex>
#include <utility>
#include <vector>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/component/optional.hpp"
#include "entity/entity.hpp"
#include "entity/entity_pool.hpp"

namespace boost {
namespace iterators {
struct forward_traversal_tag;
}  // namespace iterators
}  // namespace boost

// ----------------------------------------------------------------------------
//
namespace entity { namespace component 
{
	template<typename ComponentPool>
	class creation_queue;
	template<typename ComponentPool>
	class destruction_queue;

	template<typename T>
	class sparse_pool
	{
		template<typename ValueType>
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl<ValueType>
			  , ValueType&
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
			friend class sparse_pool;
			
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

			ValueType& dereference() const
			{
				return iterator_->second;
			}

			parent_iterator iterator_;
		};

		template<typename ValueType>
		struct optional_iterator_impl
			  : boost::iterator_facade<
			    optional_iterator_impl<ValueType>
			  , optional<ValueType>
			  , boost::forward_traversal_tag
			  , optional<ValueType>
		  	>
		{
			optional_iterator_impl()
			{}

			entity get_entity() const
			{
				return iterator_->first;
			}

			void set_target(entity e) 
			{
				entity_index_ = e.index();
				while(entity_index_ < last_entity_index_ && entity_index_ < iterator_->first.index())
					++iterator_;
			}

		private:

			friend class boost::iterator_core_access;
			friend class sparse_pool;
			
			typedef typename boost::container::flat_map<entity, T>::iterator parent_iterator;

			optional_iterator_impl(
				parent_iterator convert_from, 
				entity_index_t first_entity,
				entity_index_t last_entity)
				: iterator_(std::move(convert_from))
				, entity_index_(first_entity)
				, last_entity_index_(last_entity)
			{}

			void increment()
			{
				++entity_index_;
				while(entity_index_ < last_entity_index_ && entity_index_ < iterator_->first.index())
					++iterator_;
			}

			bool equal(optional_iterator_impl const& other) const
			{
				return entity_index_ == other.entity_index_;
			}

			optional<ValueType> dereference() const
			{
				if(entity_index_ < last_entity_index_ && entity_index_ == iterator_->first.index())
					return iterator_->second;
				else
					return boost::none;
			}

			entity_index_t entity_index_;
			entity_index_t last_entity_index_;
			parent_iterator iterator_;
		};

	public:

		typedef T type;
		typedef T value_type;
		typedef optional<T> optional_type;
		typedef iterator_impl<T> iterator;
		typedef iterator_impl<const T> const_iterator;
		typedef optional_iterator_impl<T> optional_iterator;
		typedef optional_iterator_impl<const T> const_optional_iterator;

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

			bool is_end() const
			{
				return iterator_ == end_;
			}

		private:

			friend class sparse_pool;

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
		sparse_pool(entity_pool& owner_pool, T const& default_value = T())
		{
		#if ENTITY_SUPPORT_VARIADICS
			auto create_func = &sparse_pool::create<T const&>;
		#else
			auto create_func = &sparse_pool::create;
		#endif

			// Create default values for existing entities.
			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				boost::bind(
					create_func,
					this,
					::_1,
					boost::ref(default_value)
				)
			);

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&sparse_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&sparse_pool::handle_swap_entity,
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
						&sparse_pool::create,
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
			auto r = components_.emplace(e, std::forward<Args>(args)...);
			return &(r.first->second);
		}
	#else
		T* create(entity e, type original)
		{
			auto r = components_.emplace(e, std::move(original));
			return &(r.first->second);
		}
	#endif

		void destroy(entity e)
		{
			components_.erase(e);
		}

		optional<T> get(entity e)
		{
			auto obj = components_.find(e);
			if(obj != components_.end())
			{
				return obj->second;
			}

			return boost::none;
		}

		optional<const T> get(entity e) const
		{
			auto obj = components_.find(e);
			if(obj != components_.end())
			{
				return obj->second;
			}

			return boost::none;
		}

		iterator begin()
		{
			return iterator(components_.begin());
		}

		iterator end()
		{
			return iterator(components_.end());
		}

		const_iterator begin() const
		{
			return iterator(components_.begin());
		}

		const_iterator end() const
		{
			return iterator(components_.end());
		}

		optional_iterator optional_begin()
		{
			if(components_.empty())
			{
				return optional_iterator(components_.begin(), 0, 0);
			}
			else
			{
				return optional_iterator(
					components_.begin(),
					0, 
					components_.rbegin()->first.index() + 1
				);
			}
		}

		optional_iterator optional_end()
		{
			if(components_.empty())
			{
				return optional_iterator(components_.end(), 0, 0);
			}
			else
			{
				return optional_iterator(
					components_.end(),
					components_.rbegin()->first.index() + 1,
					components_.rbegin()->first.index() + 1
				);
			}
		}

		const_optional_iterator optional_begin() const
		{
			if(components_.empty())
			{
				return const_optional_iterator(components_.begin(), 0, 0);
			}
			else
			{
				return const_optional_iterator(
					components_.begin(),
					0, 
					components_.rbegin()->first.index() + 1
				);
			}
		}

		const_optional_iterator optional_end() const
		{
			if(components_.empty())
			{
				return const_optional_iterator(components_.end(), 0, 0);
			}
			else
			{
				return const_optional_iterator(
					components_.end(),
					components_.rbegin()->first.index() + 1,
					components_.rbegin()->first.index() + 1
				);
			}
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
		
		// No copying
		sparse_pool(sparse_pool const&);
		sparse_pool operator=(sparse_pool);

		friend class creation_queue<sparse_pool<T>>;
		friend class destruction_queue<sparse_pool<T>>;

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
} } // namespace entity { namespace component 

#endif // _ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_
