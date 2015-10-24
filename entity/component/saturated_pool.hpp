// ****************************************************************************
// entity/component/saturated_pool.h
//
// Represents a component pool where the number of components
// equals the number of entities.  Allows assumptions to be
// made to optimize bookkeeping and iteration.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_
#define _ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_

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

	public:

		typedef T type;
		typedef iterator_impl<T> iterator;
		typedef iterator_impl<const T> const_iterator;

		// --------------------------------------------------------------------
		//
		struct window
		{
			typedef type value_type;

			window()
			{}

			bool is_entity(entity) const
			{
				return true;
			}

			bool increment(entity)
			{
				++data_;
				return true;
			}

			bool advance(entity e)
			{
				data_ = data_begin_ + e.index();
				return true;
			}

			value_type& get() const
			{
				return *data_;
			}

			bool is_end() const
			{
				return false;
			}

		private:

			friend class saturated_pool;

			window(saturated_pool* parent)
				: data_begin_(&parent->components_[0])
				, data_(&parent->components_[0])
			{}

			value_type* data_begin_;
			value_type* data_;
		};

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
						&saturated_pool::create,
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
			components_.emplace(components_.begin() + e.index(), std::forward<Args>(args)...);
			return &components_[e.index()];
		}	
	#else
		T* create(entity e, type original)
		{
			components_.emplace(components_.begin() + e.index(), std::move(original));
			return &components_[e.index()];
		}	
	#endif

		void destroy(entity e)
		{
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
		
		const_iterator begin() const
		{
			return const_iterator(this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(this, components_.size());
		}

		window view()
		{
			return window(this);
		}

		std::size_t size()
		{
			return components_.size();
		}

	private:

		// No copying.
		saturated_pool(saturated_pool const&);
		saturated_pool operator=(saturated_pool);

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
				create(first->first.lock().get(), std::move(first->second));
				++first;
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
} } // namespace entity { namespace component

#endif // _ENTITY_COMPONENT_SATURATEDPOOL_H_INCLUDED_
