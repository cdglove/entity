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
				return entity(std::distance(m_Begin, m_Iterator));
			}

			iterator_impl()
			{}

			void advance_to_target_entity(entity target)
			{
				m_Iterator = m_Begin + target.index();
			}

			T* maybe_extract_ptr(entity ent) const
			{
				if(!m_Parent->is_available(ent.index()))
				{
					return m_Parent->get_component(ent);
				}

				return nullptr;
			}

			bool is_valid() const
			{
				return !m_Parent->is_available(get_entity().index());
			}

		private:

			friend class boost::iterator_core_access;
			friend class dense_component_pool;
			
			typedef typename std::vector<char>::iterator parent_iterator;

			iterator_impl(dense_component_pool* parent, parent_iterator start, parent_iterator first)
				: m_Parent(parent)
				, m_Iterator(std::move(start))
				, m_Begin(std::move(first))
			{}

			void increment()
			{
				DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__iterator_impl__increment);
				++m_Iterator;
			}

			bool equal(iterator_impl const& other) const
			{
				return m_Iterator == other.m_Iterator;
			}

			T& dereference() const
			{
				return *m_Parent->get_component(get_entity());
			}

			dense_component_pool* m_Parent;
			parent_iterator m_Iterator;
			parent_iterator m_Begin;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;
		
		dense_component_pool(entity_pool const& owner_pool)
			: m_Components(new element_t[owner_pool.size()])
			, m_Available(owner_pool.size(), 1)
			, m_EntityPool(owner_pool)
		{}

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		T* create(entity e, Args&&... args)
		{
			DAILY_AUTO_INSTRUMENT_NODE(dense_component_pool__create);
			set_available(e.index(), false);
			T* ret_val = get_component(e);
			new(ret_val) T(std::forward<Args>(args)...);
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

			assert(!is_available(e.index()) && "Trying to destroy un-allocated component.");
			
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
			return iterator(this, m_Available.begin(), m_Available.begin());
		}

		iterator end()
		{
			return iterator(this, m_Available.end(), m_Available.begin());
		}

	private:

		friend class component_pool_creation_queue<dense_component_pool<type>>;
		friend class component_pool_destruction_queue<dense_component_pool<type>>;

		T* get_component(entity e)
		{
			return reinterpret_cast<T*>(&m_Components[e.index()]);
		}

		T const* get_component(entity e) const
		{
			return reinterpret_cast<T*>(&m_Components[e.index()]);
		}

		bool is_available(entity_index_t idx)
		{
			return m_Available[idx] != 0;
		}

		void set_available(entity_index_t idx, bool available)
		{
            m_Available[idx] = available == true;
		}

		template<typename Iter>
		void create_range(Iter first, Iter last)
		{
			while(first != last)
			{
				m_Available[first->first.index()] = false;
				new(get_component(first->first)) T(first->second);
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

		typedef boost::aligned_storage<
			sizeof(T), 
			boost::alignment_of<T>::value
		> element_t;

		std::unique_ptr<element_t[]>	m_Components;
		std::vector<char>				m_Available;
		entity_pool const& 				m_EntityPool;
	};
}

#endif // _ENTITY_DENSECOMPONENTPOOL_H_INCLUDED_
