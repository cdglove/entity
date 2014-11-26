//! \file entity/saturated_component_pool.h
//
// Represents a component pool where the number of components
// equals the number of entities.
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

			void advance_to_target_entity(entity e)
			{
				entity_index_ = e.index();
			}

			T* maybe_extract_ptr(entity ent) const
			{
				return parent_->get_component(ent);
			}
			
			bool is_valid()
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
		
		saturated_component_pool(entity_pool const& owner_pool)
		{}

		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			DAILY_AUTO_INSTRUMENT_NODE(saturated_component_pool__create);
			components_.emplace_back(std::forward<Args>(args)...);
			return &components_.back();
		}	

		void destroy(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(saturated_component_pool__destroy);
			using std::swap;
			swap(components_[e.index()], components_.back());
			components_.pop_back();
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

	private:

		friend class component_pool_creation_queue<saturated_component_pool<type>>;
		friend class component_pool_destruction_queue<saturated_component_pool<type>>;

		T* get_component(entity e)
		{
			return &components_[e.index()];
		}

		T const* get_component(entity e) const
		{
			return &components_[e.index()];
		}

		template<typename Iter>
		void create_range(Iter first, Iter last)
		{
			while(first != last)
			{
				components_.emplace_back(first->second);
				++first;
			}
		}

		template<typename Iter>
		void destroy_range(Iter first, Iter last)
		{
			while(first != last)
			{
				destroy(*first);
				++first;
			}
		}

		std::vector<type> components_;
	};
}

#endif // _ENTITY_SATURATEDCOMPONENTPOOL_H_INCLUDED_
