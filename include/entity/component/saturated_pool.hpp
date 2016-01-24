// ****************************************************************************
// entity/component/saturated_pool.h
//
// Represents a component pool where the number of components
// equals the number of entities.  Allows assumptions to be
// made to optimize bookkeeping and iteration.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_
#define ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_

#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <mutex>
#include <vector>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/component/required.hpp"
#include "entity/entity.hpp"
#include "entity/entity_index.hpp"
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
	class saturated_pool
	{
	private:

		template<typename ValueType>
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl<ValueType>
			  , ValueType&
			  , boost::forward_traversal_tag
		  	>
		{
			iterator_impl()
			{}

			entity get_entity() const
			{
				return make_entity(entity_index_);
			}

		private:

			friend class boost::iterator_core_access;
			friend class saturated_pool;
			
			typedef ValueType* parent_iterator;

			iterator_impl(
				saturated_pool* parent,
				entity_index_t idx)
				: parent_(parent)
				, entity_index_(idx)
			{}

			void increment()
			{
				++entity_index_;
			}

			bool equal(iterator_impl const& other) const
			{
				return entity_index_ == other.entity_index_;
			}

			ValueType& dereference() const
			{
				return parent_->components_[entity_index_];
			}

			saturated_pool* parent_;
			entity_index_t entity_index_;
		};

		// For saturated pools, the elements are never 'optional', so the name
		// optional is incorrect.  However, we want the pools to have
		// compatible interfaces so we retain the name.
		template<typename ValueType>
		struct optional_iterator_impl
			  : boost::iterator_facade<
			    optional_iterator_impl<ValueType>
			  , required<ValueType>
			  , boost::forward_traversal_tag
			  , required<ValueType>
		  	>
		{
			optional_iterator_impl()
			{}

			entity get_entity() const
			{
				return make_entity(std::distance(begin_, iterator_));
			}

			void set_target(entity e)
			{
				iterator_ = begin_ + e.index();
			}

		private:

			friend class boost::iterator_core_access;
			friend class saturated_pool;
			
			typedef typename std::vector<ValueType>::iterator parent_iterator;

			optional_iterator_impl(
				saturated_pool* parent,
				entity_index_t idx)
			{
				begin_ = parent->components_.begin();
				iterator_ = begin_ + idx;
			}

			void increment()
			{
				++iterator_;
			}

			bool equal(optional_iterator_impl const& other) const
			{
				return iterator_ == other.iterator_;
			}

			required<ValueType> dereference() const
			{
				return *iterator_;
			}

			parent_iterator begin_;
			parent_iterator iterator_;
		};

	public:

		typedef T type;
		typedef T value_type;
		typedef required<T> optional_type;
		typedef required<T const> const_optional_type;
		typedef iterator_impl<T> iterator;
		typedef iterator_impl<T const> const_iterator;
		typedef optional_iterator_impl<T> optional_iterator;
		typedef optional_iterator_impl<T const> const_optional_iterator;

		// --------------------------------------------------------------------
		//		
		saturated_pool(entity_pool& owner_pool, T const& default_value = T())
		{
			components_.resize(owner_pool.size(), default_value);

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&saturated_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&saturated_pool::handle_swap_entity,
						this,
						::_1, 
						::_2
					)
				)
			;

			auto_create_components(owner_pool, default_value);
		}

		template<typename... Args>
		void auto_create_components(entity_pool& owner_pool, Args... constructor_args)
		{
			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					std::function<void(entity)>(
						[this, constructor_args...](entity e) 
						{
                            create_impl(e, constructor_args...);
						}
					)
				)
			;
		}

		// Saturated pools cant create or destroy things independently 
		// of the entity pool, so these functions should not exist.
		// Right now they're here for compatibility.
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			return &components_[e.index()];
		}	

		void destroy(entity e)
		{}

		required<T> get(entity e)
		{
			return *get_component(e);
		}

		required<T const> get(entity e) const
		{
			return *get_component(e);
		}

		iterator begin()
		{
			return iterator(this, 0);
		}

		iterator end()
		{
			return iterator(this, components_.size());
		}
		
		const_iterator begin() const
		{
			return const_iterator(this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(this, components_.size());
		}

		optional_iterator optional_begin()
		{
			return optional_iterator(this, 0);
		}

		optional_iterator optional_end()
		{
			return optional_iterator(this, components_.size());
		}
		
		const_optional_iterator optional_begin() const
		{
			return const_optional_iterator(this, 0);
		}

		const_optional_iterator optional_end() const
		{
			return const_optional_iterator(this, components_.size());
		}

		std::size_t size()
		{
			return components_.size();
		}

	private:

		// No copying.
		saturated_pool(saturated_pool const&);
		saturated_pool operator=(saturated_pool);

		// Saturated pools cant create or destroy things independently 
		// of the entity pool, so such functions should be private.
		template<typename... Args>
		T* create_impl(entity e, Args&&... args)
		{
			components_.emplace(components_.begin() + e.index(), std::forward<Args>(args)...);
			return &components_[e.index()];
		}	

		void destroy_impl(entity e)
		{
			components_.erase(components_.begin() + e.index());
		}

		friend class creation_queue<saturated_pool<T>>;
		friend class destruction_queue<saturated_pool<T>>;

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
				create_impl(first->first.lock().get(), std::move(first->second));
				++first;
			}
		}

		template<typename Iter>
		void destroy_range(Iter current, Iter last)
		{
			while(current != last)
			{
				destroy_impl(current->lock().get());
				++current;
			}
		}

		// --------------------------------------------------------------------
		// Slot Handlers.
		void handle_destroy_entity(entity e)
		{
			destroy_impl(e);
		}

		void handle_swap_entity(entity a, entity b)
		{
			using std::swap;
			swap(components_[a.index()], components_[b.index()]);
		}

		std::vector<type> components_;
		slot_list		  slots_;
	};
} } // namespace entity { namespace component

#endif // ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_
