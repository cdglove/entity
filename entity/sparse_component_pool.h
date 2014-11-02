//! \file entity/dense_component_pool.h
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
#pragma once
#ifndef _ENTITY_SPARSECOMPONENT_POOL_H_INCLUDED_
#define _ENTITY_SPARSECOMPONENT_POOL_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_pool.h"
#include "entity/entity_component_iterator.h"
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/container/flat_map.hpp>
#include <cstddef>
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
	class sparse_component_pool
	{
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			entity	get_entity() const
			{
				return m_Iterator->first;
			}

			iterator_impl()
			{}

			void advance(entity target)
			{
				while(get_entity() < target)
				{
					++m_Iterator;
				}
			}

		private:

			friend class boost::iterator_core_access;
			friend class sparse_component_pool;
			
			typedef typename boost::container::flat_map<entity, T>::iterator parent_iterator;


			explicit iterator_impl(parent_iterator convert_from)
				: m_Iterator(std::move(convert_from))
			{}

			void increment()
			{
				DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__iterator_impl__increment);
				++m_Iterator;
			}

			bool equal(iterator_impl const& other) const
			{
				return m_Iterator == other.m_Iterator;
			}

			T& dereference() const
			{
				return m_Iterator->second;
			}

			parent_iterator m_Iterator;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;

		friend class component_pool_creation_queue<sparse_component_pool<type>>;
		friend class component_pool_destruction_queue<sparse_component_pool<type>>;

		sparse_component_pool(entity_pool const& owner_pool)
		{
			// Inserta dummy node to make iteration simpler.
			m_Entities[entity(owner_pool.size())] = T();
		}

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__create);
			auto r = m_Entities.emplace(e, std::forward<Args>(args)...);
			return &(r.first->second);
		}
	#else
		T* create(entity e, type&& original)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__create);
			auto r = m_Entities.emplace(e, std::move(original));
			return &(r.first->second);
		}
	#endif

		void destroy(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__destroy);
			m_Entities.erase(e);
		}

		T* get(entity e)
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__get);
			auto obj = m_Entities.find(e);
			if(obj != m_Entities.end())
			{
				return &obj->second;
			}

			return nullptr;
		}

		T const* get(entity e) const
		{
			DAILY_AUTO_INSTRUMENT_NODE(sparse_component_pool__get);
			auto obj = m_Entities.find(e);
			if(obj != m_Entities.end())
			{
				return &obj->second;
			}

			return nullptr;
		}

		iterator begin()
		{
			return iterator(m_Entities.begin());
		}

		iterator end()
		{
			return iterator(m_Entities.end()-1);
		}

	private:

		template<typename Iter>
		void create_range(Iter first, Iter last)
		{
			m_Entities.insert(boost::container::ordered_unique_range_t(), first, last);
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

		boost::container::flat_map<entity, T> m_Entities;
	};
}

#endif // _ENTITY_DENSECOMPONENT_POOL_H_INCLUDED_
