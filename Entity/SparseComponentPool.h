//! \file Entity/DenseComponentPool.h
//
// Represents a component pool where the number of components
// approaches the number of entitys on those components.
// Basically trades space for run time efficiency.
// 
#pragma once
#ifndef _COMPONENT_SPARSECOMPONENT_POOL_H_INCLUDED_
#define _COMPONENT_SPARSECOMPONENT_POOL_H_INCLUDED_

#include "Entity/EntityPool.h"
#include "Entity/EntityComponentIterator.h"
#include <boost/type_traits/aligned_storage.hpp>
#include <cstddef>
#include <boost/container/flat_map.hpp>

// ----------------------------------------------------------------------------
//
namespace Entity 
{
	template<typename T>
	class SparseComponentPool
	{
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , T&
			  , boost::forward_traversal_tag
		  	>
		{
			Entity	get_entity() const
			{
				return m_Iterator->first;
			}

			iterator_impl()
			{}

		private:

			friend class boost::iterator_core_access;
			friend class SparseComponentPool;
			
			typedef typename boost::container::flat_map<Entity, T>::iterator parent_iterator;



			explicit iterator_impl(parent_iterator convert_from)
				: m_Iterator(std::move(convert_from))
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
				return m_Iterator->second;
			}

			parent_iterator m_Iterator;
		};

	public:

		typedef T type;
		typedef iterator_impl iterator;

		SparseComponentPool(EntityPool const& owner_pool)
			: m_EntityPool(owner_pool)
		{}

		T* create(Entity e)
		{
			return &m_Entities[e];
		}	

		void destroy(Entity e)
		{
			m_Entities.erase(e);
		}

		T* get(Entity e)
		{
			auto obj = m_Entities.find(e);
			if(obj != m_Entities.end())
			{
				return &obj->second;
			}

			return nullptr;
		}

		T const* get(Entity e) const
		{
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
			return iterator(m_Entities.end());
		}

	private:

		boost::container::flat_map<Entity, T> m_Entities;
		EntityPool const& 					   m_EntityPool;
	};
}

#endif // _COMPONENT_DENSECOMPONENT_POOL_H_INCLUDED_