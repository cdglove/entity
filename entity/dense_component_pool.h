//! \file entity/dense_component_pool.h
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
#pragma once
#ifndef _COMPONENT_DENSECOMPONENTPOOL_H_INCLUDED_
#define _COMPONENT_DENSECOMPONENTPOOL_H_INCLUDED_

#include "entity/entity_pool.h"
#include "entity/entity_component_iterator.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <cstddef>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------
//
namespace entity 
{
	template<typename T>
	class dense_component_pool
	{
	private:

		// struct iterator_impl
		// 	  : boost::iterator_facade<
		// 	    iterator_impl
		// 	  , T&
		// 	  , boost::forward_traversal_tag
		//   	>
		// {
		// 	iterator_impl()
		// 		: m_Parent(nullptr)
		// 		, m_Index(0)
		// 	{}

		// 	entity get_entity() const
		// 	{
		// 		return entity(m_Index);
		// 	}	

		// private:

		// 	friend class boost::iterator_core_access;
		// 	friend class dense_component_pool;

		// 	iterator_impl(dense_component_pool* parent, std::size_t index)
		// 		: m_Parent(parent)
		// 		, m_Index(index)
		// 	{}	

		// 	void increment()
		// 	{
		// 		while(++m_Index < m_Parent->m_EntityPool.size() && m_Parent->is_available(m_Index))
		// 			;
		// 	}

		// 	bool equal(iterator_impl const& other) const
		// 	{
		// 		return m_Index == other.m_Index;
		// 	}

		// 	T& dereference() const
		// 	{
		// 		return *m_Parent->get_component(entity(m_Index));
		// 	}

		// 	std::size_t m_Index;
		// 	dense_component_pool* m_Parent;
		// };

		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			entity get_entity() const
			{
				return *m_Iterator;
			}

			iterator_impl()
			{}

		private:

			friend class boost::iterator_core_access;
			friend class dense_component_pool;
			
			typedef typename std::vector<entity>::iterator parent_iterator;


			explicit iterator_impl(dense_component_pool* parent, parent_iterator convert_from)
				: m_Parent(parent)
				, m_Iterator(std::move(convert_from))
			{}

			void increment()
			{
				++m_Iterator;
			}

			bool equal(iterator_impl const& other) const
			{
				return m_Iterator == other.m_Iterator;
			}

			T& dereference() const
			{
				return *m_Parent->get_component(*m_Iterator);
			}

			dense_component_pool* m_Parent;
			parent_iterator m_Iterator;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;
		
		dense_component_pool(entity_pool const& owner_pool)
			: m_Components(new element_t[owner_pool.size()])
			, m_Available(owner_pool.size(), 1)
			, m_EntityPool(owner_pool)
		{
			m_Entitys.reserve(owner_pool.size());
		}

		T* create(entity e)
		{
			set_available(e, false);
			T* ret_val = get_component(e);
			new(ret_val) T();
			return ret_val;
		}	

		void destroy(entity e)
		{
			std::size_t obj_index = e;

			assert(obj_index < m_EntityPool.size() && "Trying to destroy component not owned by this pool");
			assert(!is_available(obj_index) && "Trying to destroy un-allocated component.");
			
			T* p = get_component(e);
			p->~T();
			
			set_available(obj_index, true);
		}

		T* get(entity e)
		{
			if(is_available(e))
			{
				return nullptr;
			}

			return get_component(e);
		}

		T const* get(entity e) const
		{
			if(is_available(e))
			{
				return nullptr;
			}

			return get_component(e);
		}

		iterator begin()
		{
			return iterator(this, m_Entitys.begin());
		}

		iterator end()
		{
			return iterator(this, m_Entitys.end());
		}

	private:

		T* get_component(entity e)
		{
			std::size_t index = e;
			return reinterpret_cast<T*>(&m_Components[index]);
		}

		T const* get_component(entity e) const
		{
			std::size_t index = e;
			return reinterpret_cast<T*>(&m_Components[index]);
		}

		bool is_available(std::size_t e)
		{
			return m_Available[e];
		}

		void set_available(std::size_t e, bool available)
		{
			auto position = std::lower_bound(m_Entitys.begin(), m_Entitys.end(), entity(e));
			if(available)
			{
				m_Entitys.erase(position);
			}
			else
			{
				m_Entitys.insert(position, entity(e));
			}
			m_Available[e] = available;
		}

		typedef boost::aligned_storage<
			sizeof(T), 
			boost::alignment_of<T>::value
		> element_t;

		std::unique_ptr<element_t[]>	m_Components;
		std::vector<entity> 			m_Entitys;
		std::vector<char>				m_Available;
		entity_pool const& 				m_EntityPool;
	};
}

#endif // _COMPONENT_DENSECOMPONENTPOOL_H_INCLUDED_