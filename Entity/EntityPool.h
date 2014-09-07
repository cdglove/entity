//! \file Entity/EntityPool.h
//
// Represents a pool of entity ids.
// 
#pragma once
#ifndef _COMPONENT_ENTITYPOOL_H_INCLUDED_
#define _COMPONENT_ENTITYPOOL_H_INCLUDED_

#include "Entity/Entity.h"
#include <boost/container/flat_set.hpp>
#include <vector>
#include <cassert>

// ----------------------------------------------------------------------------
//
namespace Entity
{
	class EntityPool
	{
	public:

		typedef boost::container::flat_set<Entity>::iterator iterator;
		typedef boost::container::flat_set<Entity>::const_iterator const_iterator;

		EntityPool(std::size_t max_entitys)
			: m_FreeEntitys(max_entitys)
			, m_Size(max_entitys)
		{
			for(std::size_t i = 0; i < max_entitys; ++i)
			{
				m_FreeEntitys.push_back(Entity(max_entitys-i-1));
			}	

			m_UsedEntitys.reserve(max_entitys);		
		}

		Entity create()
		{
			auto ret_val = m_FreeEntitys.back();
			m_FreeEntitys.pop_back();
			m_UsedEntitys.insert(ret_val);
			return ret_val;
		}	

		void destroy(Entity e)
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

		EntityPool(EntityPool const&);
		EntityPool operator=(EntityPool);

		std::vector<Entity>     			m_FreeEntitys;
		boost::container::flat_set<Entity> m_UsedEntitys;
		std::size_t 						m_Size;
	};

	EntityPool::iterator begin(EntityPool& p)
	{
		return p.begin();
	}

	EntityPool::const_iterator begin(EntityPool const& p)
	{
		return p.begin();
	}

	EntityPool::const_iterator cbegin(EntityPool const& p)
	{
		return p.cbegin();
	}

	EntityPool::iterator end(EntityPool& p)
	{
		return p.end();
	}

	EntityPool::const_iterator end(EntityPool const& p)
	{
		return p.end();
	}

	EntityPool::const_iterator cend(EntityPool const& p)
	{
		return p.cend();
	}
}

#endif // _COMPONENT_ENTITYPOOL_H_INCLUDED_