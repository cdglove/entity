// ****************************************************************************
// entity/component/dense_pool.hpp
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_COMPONENT_DENSEPOOL_H_INCLUDED_
#define _ENTITY_COMPONENT_DENSEPOOL_H_INCLUDED_

#include <boost/assert.hpp>
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/ref.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <mutex>
#include <new>
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
	class dense_pool
	{
	private:

		struct element_t
		{
			char mem_[sizeof(T)];
		};

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

		private:

			friend class boost::iterator_core_access;
			friend class dense_pool;

			typedef typename std::vector<char>::iterator parent_iterator;

			iterator_impl(dense_pool* parent, entity_index_t start)
				: parent_(parent)
				, entity_index_(start)
			{
				// Fast forward to the first available.
				auto available_iterator = parent_->available_.begin() + start;
				while(entity_index_ < parent_->available_.size() && *available_iterator)
				{
					++available_iterator;
					++entity_index_;
				}
			}

			void increment()
			{
				auto available_iterator = parent_->available_.begin() + entity_index_;
				auto end_iterator = parent_->available_.end();
				if(available_iterator != end_iterator)
					++entity_index_;
				while(available_iterator != end_iterator && *available_iterator)
				{
					++available_iterator;
					++entity_index_;
				}
			}

			bool equal(iterator_impl const& other) const
			{
				return entity_index_ == other.entity_index_;
			}

			T& dereference() const
			{
				return *parent_->get_component(get_entity().index());
			}

			dense_pool* parent_;
			entity_index_t entity_index_;
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

			bool is_entity(entity) const
			{
				return !(*available_);
			}

			bool increment(entity)
			{
				++data_;
				++available_;
				return !(*available_);
			}

			bool advance(entity e)
			{
				data_ = data_begin_ + e.index();
				available_ = available_begin_ + e.index();
				return !(*available_);
			}

			value_type& get() const
			{
				return *reinterpret_cast<value_type*>(data_);
			}

			bool is_end() const
			{
				return false;
			}

		private:

			friend class dense_pool;

			window(dense_pool* parent)
				: available_begin_(&parent->available_[0])
				, available_(&parent->available_[0])
				, data_begin_(&parent->components_[0])
				, data_(&parent->components_[0])
			{}

			char const* available_begin_;
			char const* available_;
			typename dense_pool::element_t* data_begin_;
			typename dense_pool::element_t* data_;
		};
			
		// --------------------------------------------------------------------
		//
		dense_pool(entity_pool& owner_pool, T const& default_value = T())
			: used_count_(0)
		{
			components_.resize(owner_pool.size());
			available_.resize(owner_pool.size(), true);

		#if ENTITY_SUPPORT_VARIADICS
			auto create_func = &dense_pool::create<T const&>;
		#else
			auto create_func = &dense_pool::create;
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

			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					boost::bind(
						&dense_pool::handle_create_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					boost::bind(
						&dense_pool::handle_destroy_entity,
						this,
						::_1
					)
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					boost::bind(
						&dense_pool::handle_swap_entity,
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
					std::function<void(entity)>(
						[this, default_value](entity e)
						{
							create_entity_slot(e);
							create(e, default_value);
						}
					)
				)
			;
		}
	#endif

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			set_available(e.index(), false);
			T* ret_val = get_component(e.index());
			new(ret_val) T(std::forward<Args>(args)...);
			++used_count_;
			return ret_val;
		}	
	#else
		T* create(entity e, type original)
		{
			set_available(e.index(), false);
			T* ret_val = get_component(e.index());
			new(ret_val) T(std::move(original));
			++used_count_;
			return ret_val;
		}	
	#endif

		void destroy(entity e)
		{
			BOOST_ASSERT(!is_available(e.index()) && "Trying to destroy un-allocated component.");
			--used_count_;
			T* p = get_component(e.index());
			p->~T();
			
			set_available(e.index(), true);
		}

		T* get(entity e)
		{
			if(is_available(e.index()))	
			{
				return nullptr;
			}

			return get_component(e.index());
		}

		T const* get(entity e) const
		{
			if(is_available(e.index()))
			{
				return nullptr;
			}

			return get_component(e.index());
		}

		iterator begin()
		{
			return iterator(this, 0);
		}

		iterator end()
		{
			return iterator(this, available_.size());
		}

		window view()
		{
			return window(this);
		}

		std::size_t size()
		{
			return used_count_;
		}

	private:

		// No copying
		dense_pool(dense_pool const&);
		dense_pool operator=(dense_pool);

		friend class creation_queue<dense_pool<T>>;
		friend class destruction_queue<dense_pool<T>>;

		struct slot_list
		{
			boost::signals2::scoped_connection entity_create_handler;
			boost::signals2::scoped_connection entity_destroy_handler;
			boost::signals2::scoped_connection entity_swap_handler;
		};

		T* get_component(entity_index_t e)
		{
			return reinterpret_cast<T*>(&components_[e]);
		}

		T const* get_component(entity_index_t e) const
		{
			return reinterpret_cast<T*>(&components_[e]);
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

			auto c_a = get(a);
			auto c_b = get(b);

			if(c_a && c_b)
			{
				swap(*c_a, *c_b);
			}
			else if(c_a)
			{
				create(b, std::move(*c_a));
				destroy(a);
			}
			else if(c_b)
			{
				create(a, std::move(*c_b));
				destroy(b);
			}
		}

		std::vector<element_t>			components_;
		std::vector<char>				available_;
		std::size_t						used_count_;
		slot_list						slots_;
	};
} } // namespace entity { namespace component
	
#endif // _ENTITY_COMPONENT_DENSEPOOL_H_INCLUDED_