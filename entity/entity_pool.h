//! \file entity/entity_pool.h
//
// Represents a pool of entity ids.
// 
#pragma once
#ifndef _COMPONENT_ENTITYPOOL_H_INCLUDED_
#define _COMPONENT_ENTITYPOOL_H_INCLUDED_

#include "entity/entity.h"
#include <boost/container/flat_set.hpp>
#include <vector>
#include <cassert>

// ----------------------------------------------------------------------------
//
namespace entity
{
	class entity_pool
	{
	public:

		typedef boost::container::flat_set<entity>::iterator iterator;
		typedef boost::container::flat_set<entity>::const_iterator const_iterator;

		entity_pool(std::size_t max_entitys)
			: m_FreeEntitys(max_entitys)
			, m_Size(max_entitys)
		{
			for(std::size_t i = 0; i < max_entitys; ++i)
			{
				m_FreeEntitys.push_back(entity(max_entitys-i-1));
			}	

			m_UsedEntitys.reserve(max_entitys);		
		}

		entity create()
		{
			auto ret_val = m_FreeEntitys.back();
			m_FreeEntitys.pop_back();
			m_UsedEntitys.insert(ret_val);
			return ret_val;
		}	

		void destroy(entity e)
		{
			assert(e < m_Size && "Trying to return an entity not from this pool");
			m_UsedEntitys.erase(e);
			m_FreeEntitys.push_back(e);
		}

		std::size_t size() const
		{
			return m_Size;
		}

		iterator begin()
		{
			return m_UsedEntitys.begin();
		}

		const_iterator begin() const
		{
			return m_UsedEntitys.begin();
		}
		
		const_iterator cbegin() const
		{
			return m_UsedEntitys.cbegin();
		}

		iterator end()
		{
			return m_UsedEntitys.end();
		}

		const_iterator end() const
		{
			return m_UsedEntitys.end();
		}

		const_iterator cend() const
		{
			return m_UsedEntitys.cend();
		}

	private:

		entity_pool(entity_pool const&);
		entity_pool operator=(entity_pool);

		std::vector<entity>     			m_FreeEntitys;
		boost::container::flat_set<entity> m_UsedEntitys;
		std::size_t 						m_Size;
	};

	entity_pool::iterator begin(entity_pool& p)
	{
		return p.begin();
	}

	entity_pool::const_iterator begin(entity_pool const& p)
	{
		return p.begin();
	}

	entity_pool::const_iterator cbegin(entity_pool const& p)
	{
		return p.cbegin();
	}

	entity_pool::iterator end(entity_pool& p)
	{
		return p.end();
	}

	entity_pool::const_iterator end(entity_pool const& p)
	{
		return p.end();
	}

	entity_pool::const_iterator cend(entity_pool const& p)
	{
		return p.cend();
	}
}

#endif // _COMPONENT_ENTITYPOOL_H_INCLUDED_