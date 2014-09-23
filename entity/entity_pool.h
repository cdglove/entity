//! \file entity/entity_pool.h
//
// Represents a pool of entity ids.
// 
#pragma once
#ifndef _COMPONENT_ENTITYPOOL_H_INCLUDED_
#define _COMPONENT_ENTITYPOOL_H_INCLUDED_

#include "entity/entity.h"
#include <boost/iterator/iterator_facade.hpp>
#include <vector>
#include <algorithm>
#include <cassert>

// ----------------------------------------------------------------------------
//
namespace entity
{
	class entity_pool
	{
		struct iterator_impl
			  : boost::iterator_facade<
			    iterator_impl
			  , entity
			  , boost::forward_traversal_tag
			  , entity
		  	>
		{
		private:

			friend class boost::iterator_core_access;
			friend class entity_pool;
			
			typedef typename std::vector<char>::const_iterator parent_iterator;

			iterator_impl(parent_iterator first, parent_iterator convert_from)
				: m_Begin(std::move(first))
				, m_Iterator(std::move(convert_from))
			{}

			void increment()
			{
				++m_Iterator;
				while(*m_Iterator)
				 	++m_Iterator;
			}

			bool equal(iterator_impl const& other) const
			{
				return m_Iterator == other.m_Iterator;
			}

			entity dereference() const
			{
				return entity(std::distance(m_Begin, m_Iterator));
			}

			parent_iterator m_Iterator;
			parent_iterator m_Begin;
		};

	public:

		typedef iterator_impl iterator;
		typedef iterator_impl const_iterator;

		entity_pool(std::size_t max_entitys)
			: m_Available(max_entitys, 1)
		{
			m_FreeList.reserve(max_entitys);
			for(std::size_t i = 0; i < max_entitys; ++i)
			{
				m_FreeList.push_back(entity(max_entitys-i-1));
			}
		}

		entity create()
		{
			entity e = m_FreeList.back();
			m_Available[e] = false;
			m_FreeList.pop_back();
			return e;
		}	

		void destroy(entity e)
		{
			m_Available[e] = true;
			m_FreeList.push_back(e);
		}

		std::size_t size() const
		{
			return m_Available.size();
		}

		iterator begin() const
		{
			return iterator_impl(m_Available.begin(), m_Available.begin());
		}

		// const_iterator begin() const
		// {
		// 	return m_UsedEntitys.begin();
		// }
		
		// const_iterator cbegin() const
		// {
		// 	return m_UsedEntitys.cbegin();
		// }

		iterator end() const
		{
			return iterator_impl(m_Available.begin(), m_Available.end());
		}

		// const_iterator end() const
		// {
		// 	return m_UsedEntitys.end();
		// }

		// const_iterator cend() const
		// {
		// 	return m_UsedEntitys.cend();
		// }

	private:

		entity_pool(entity_pool const&);
		entity_pool operator=(entity_pool);

		std::vector<char>   m_Available; // avoid vector<bool>
		std::vector<entity> m_FreeList;
	};

	entity_pool::iterator begin(entity_pool const& p)
	{
		return p.begin();
	}

	// entity_pool::const_iterator begin(entity_pool const& p)
	// {
	// 	return p.begin();
	// }

	// entity_pool::const_iterator cbegin(entity_pool const& p)
	// {
	// 	return p.cbegin();
	// }

	entity_pool::iterator end(entity_pool const& p)
	{
		return p.end();
	}

	// entity_pool::const_iterator end(entity_pool const& p)
	// {
	// 	return p.end();
	// }

	// entity_pool::const_iterator cend(entity_pool const& p)
	// {
	// 	return p.cend();
	// }
}

#endif // _COMPONENT_ENTITYPOOL_H_INCLUDED_