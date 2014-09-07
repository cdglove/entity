//! \file Entity/DenseComponentPool.h
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
#pragma once
#ifndef _COMPONENT_DENSECOMPONENT_POOL_H_INCLUDED_
#define _COMPONENT_DENSECOMPONENT_POOL_H_INCLUDED_

#include "Entity/EntityPool.h"
#include "Entity/EntityComponentIterator.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <cstddef>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------
//
namespace Entity 
{
	template<typename T>
	class DenseComponentPool
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
				: m_Parent(nullptr)
				, m_Index(-1)
			{}

			explicit iterator_impl(DenseComponentPool* parent)
				: m_Parent(parent)
				, m_Index(-1)
			{
				increment();
			}	

			Entity	get_entity() const
			{
				return m_Index;
			}	

		private:

			friend class boost::iterator_core_access;

			void increment()
			{
				while(++m_Index < m_Parent->m_Available.size() && m_Parent->m_Available[m_Index])
					;

				if(m_Index == m_Parent->m_Available.size())
					m_Index = -1;
			}

			bool equal(iterator_impl const& other) const
			{
				return m_Index == other.m_Index;
			}

			T& dereference() const
			{
				return *m_Parent->get_component(m_Index);
			}

			Entity m_Index;
			DenseComponentPool* m_Parent;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;
		
		DenseComponentPool(EntityPool const& owner_pool)
			: m_Entitys(new element_t[owner_pool.size()])
			, m_Available(owner_pool.size(), 1)
			, m_EntityPool(owner_pool)
		{}

		T* create(Entity e)
		{
			m_Available[e] = false;
			T* ret_val = get_component(e);
			new(ret_val) T();
			return ret_val;
		}	

		void destroy(Entity e)
		{
			std::size_t obj_index = e;

			assert(obj_index < m_Available.size() && "Trying to destroy component not owned by this pool");
			assert(!m_Available[obj_index] && "Trying to destroy un-allocated component.");
			
			T* p = get_component(e);
			p->~T();
			
			m_Available[obj_index] = true;
		}

		T* get(Entity e)
		{
			if(m_Available[e])
			{
				return nullptr;
			}

			return get_component(e);
		}

		T const* get(Entity e) const
		{
			if(m_Available[e])
			{
				return nullptr;
			}

			return get_component(e);
		}

		iterator begin()
		{
			return iterator(this);
		}

		iterator end()
		{
			return iterator();
		}

	private:

		T* get_component(Entity e)
		{
			std::size_t index = e;
			return reinterpret_cast<T*>(&m_Entitys[index]);
		}

		T const* get_component(Entity e) const
		{
			std::size_t index = e;
			return reinterpret_cast<T*>(&m_Entitys[index]);
		}

		typedef boost::aligned_storage<
			sizeof(T), 
			boost::alignment_of<T>::value
		> element_t;

		std::unique_ptr<element_t[]>	m_Entitys;
		std::vector<char> 				m_Available; // Avoid vector bool.
		EntityPool const& 				m_EntityPool;
	};
}

#endif // _COMPONENT_DENSECOMPONENT_POOL_H_INCLUDED_