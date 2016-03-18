// ****************************************************************************
// entity/component/sparse_pool.h
//
// Represents a component pool where the number of components
// is significantly less than the number of entities with those components.
// Uses less space and will be faster to iterate under these conditions.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_
#define ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_

#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
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
#include "entity/support/mutable_pair.hpp"

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
		typedef std::vector<entity_index_t> index_table_t;
		typedef std::vector<T> component_table_t;

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

		private:

			friend class boost::iterator_core_access;
			friend class sparse_pool;
			
			typedef typename sparse_pool<T>::component_table_t::iterator parent_iterator;

			explicit iterator_impl(parent_iterator table_iter)
				: iterator_(std::move(table_iter))
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
				return *iterator_;
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

		private:

			friend class boost::iterator_core_access;
			friend class sparse_pool;
			
			typedef typename sparse_pool<T>::index_table_t::iterator parent_iterator;
			typedef typename sparse_pool<T>::component_table_t component_table_t;

			optional_iterator_impl(
				parent_iterator table_iter, 
				component_table_t& components)
				: iterator_(std::move(table_iter))
				, components_(&components)
			{}

			void increment()
			{
				++iterator_;
			}

			bool equal(optional_iterator_impl const& other) const
			{
				return iterator_ == other.iterator_;
			}

			optional<ValueType> dereference() const
			{
				if(*iterator_ != sparse_pool<T>::no_component_flag())
					return (*components_)[*iterator_];
				else
					return boost::none;
			}

			parent_iterator iterator_;
			component_table_t* components_;
		};

	public:

		typedef T type;
		typedef T value_type;
		typedef optional<T> optional_type;
		typedef optional<T const> const_optional_type;
		typedef iterator_impl<T> iterator;
		typedef iterator_impl<T const> const_iterator;
		typedef optional_iterator_impl<T> optional_iterator;
		typedef optional_iterator_impl<T const> const_optional_iterator;
		
		// --------------------------------------------------------------------
		//
		template<typename... Args>
		sparse_pool(entity_pool& owner_pool, Args&&... args)
		{
			// Create default values for existing entities.
			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				[this](entity e)
				{
					handle_create_entity(e);
				}
			);

			std::for_each(
				owner_pool.begin(),
				owner_pool.end(),
				[&args..., this](entity e)
				{
					create(e, std::forward<Args>(args)...);
				}
			);

			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					[this](entity e)
					{
						handle_create_entity(e);
					}
				)
			;

			slots_.entity_destroy_handler = 
				owner_pool.signals().on_entity_destroy.connect(
					[this](entity e)
					{
						handle_destroy_entity(e);
					}
				)
			;

			slots_.entity_swap_handler = 
				owner_pool.signals().on_entity_swap.connect(
					[this](entity a, entity b)
					{
						handle_swap_entity(a, b);
					}
				)
			;
		}

		template<typename... Args>
		void auto_create_components(entity_pool& owner_pool, Args... args)
		{
			slots_.entity_create_handler = 
				owner_pool.signals().on_entity_create.connect(
					std::function<void(entity)>(
						[this, args...](entity e)
						{
							handle_create_entity(e);
							create(e, args...);
						}
					)
				)
			;
		}
	
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			table_[e.index()] = components_.size();
			components_.emplace_back(std::forward<Args>(args)...);
			reverse_table_.emplace_back(e.index());
			return std::addressof(components_.back());
		}
	
		void destroy(entity e)
		{
			auto idx = get_index_for_entity(e);
			table_[e.index()] = no_component_flag();
			using std::swap;
			swap(components_[idx], components_.back());
			components_.pop_back();
			reverse_table_[idx] = reverse_table_.back();
			reverse_table_.pop_back();
		}

		optional<T> get(entity e)
		{
			auto idx = get_index_for_entity(e);
			if(idx != no_component_flag())
				return components_[idx];
			return boost::none;
		}

		optional<const T> get(entity e) const
		{
			auto idx = get_index_for_entity(e);
			if(idx != no_component_flag())
				return components_[idx];
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
			return const_iterator(components_.begin());
		}

		const_iterator end() const
		{
			return const_iterator(components_.end());
		}

		optional_iterator optional_begin()
		{
			return optional_iterator(table_.begin(), components_);
		}

		optional_iterator optional_end()
		{
			return optional_iterator(table_.end(), components_);
		}

		const_optional_iterator optional_begin() const
		{
			return const_optional_iterator(table_.begin(), components_);
		}

		const_optional_iterator optional_end() const
		{
			return const_optional_iterator(table_.end(), components_);
		}

		std::size_t size()
		{
			return components_.size();
		}

	private:

		static entity_index_t no_component_flag()
		{
			return std::numeric_limits<entity_index_t>::max();
		}

		entity_index_t get_index_for_entity(entity e) const
		{
			return table_[e.index()];
		}

		
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
			auto const initial_count = components_.size();

			std::transform(first, last, std::back_inserter(components_), 
				[](std::pair<weak_entity, type>& h)
				{
					return std::move(h.second);
				}
			);

			auto current_index = initial_count;
			for(auto i = first; i != last; ++i)
			{
				auto entity = i->first.lock();
				auto entity_idx = entity.get().index();
				reverse_table_[current_index] = entity_idx;
				table_[entity_idx] = current_index++;
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
		void handle_create_entity(entity e)
		{
			if(table_.size() <= e.index())
				table_.resize(e.index()+1, no_component_flag());
		}

		void handle_destroy_entity(entity e)
		{
			auto idx = get_index_for_entity(e);
			if(idx != no_component_flag())
			{
				destroy(e);
			}
		}

		void handle_swap_entity(entity a, entity b)
		{
			using std::swap;

			// Could also just swap the indices here.
			auto idx_a = table_[a.index()];
			auto idx_b = table_[b.index()];
			swap(components_[idx_a], components_[idx_b]);
		}

		index_table_t table_;
		index_table_t reverse_table_;
		std::vector<T> components_;
		slot_list slots_;
	};
} } // namespace entity { namespace component 

#endif // ENTITY_COMPONENT_SPARSEPOOL_H_INCLUDED_
