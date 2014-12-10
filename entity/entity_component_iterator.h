//! \file entity/entity_component_iterator.h
//
// Represents an iterator that ties a component to an
// entity pool.  Allows us to iterate through an
// entity pool and component pool at the same time.
// 
#pragma once
#ifndef _ENTITY_ENTITYCOMPONENTITERATOR_H_INCLUDED_
#define _ENTITY_ENTITYCOMPONENTITERATOR_H_INCLUDED_

#include "entity/config.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <daily/timer/instrument.h>
#include <boost/typeof/typeof.hpp>

// ----------------------------------------------------------------------------
//
namespace entity 
{
	namespace detail
	{
		template<typename EntityListIterator, typename ComponentPool>
		struct extract_iterator
		{
			typedef typename 
			std::decay<
				ComponentPool
			>::type::template entity_iterator<EntityListIterator> type;
		};

		template<typename EntityList, typename ComponentPoolsTuple>
		struct component_iterators
		{
			typedef decltype(boost::declval<const EntityList>().begin()) entity_iterator;
			typedef typename
			boost::mpl::transform<
				ComponentPoolsTuple, 
				extract_iterator<
					entity_iterator, boost::mpl::_1
				>
			>::type type;
		};

		// --------------------------------------------------------------------
		//
		template<typename EntityListIterator>
		struct begin_pool
		{
			begin_pool(EntityListIterator iter)
				: entity_iter_(std::move(iter))
			{}

			template<typename Pool>
			typename Pool::template entity_iterator<EntityListIterator> operator()(Pool& p) const
			{
				return p.begin(entity_iter_);
			}

			EntityListIterator entity_iter_;
		};

		template<typename EntityListIterator>
		struct end_pool
		{
			end_pool(EntityListIterator iter)
				: entity_iter_(std::move(iter))
			{}

			template<typename Pool>
			typename Pool::template entity_iterator<EntityListIterator> operator()(Pool& p) const
			{
				return p.end(entity_iter_);
			}

			EntityListIterator entity_iter_;
		};

		template<typename EntityList, typename EntityListIterator, typename ComponentPools>
		typename component_iterators<
			EntityList, ComponentPools
		>::type	make_component_begins(EntityListIterator entity_iter, ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, begin_pool<EntityListIterator>(entity_iter));
		}

		template<typename EntityList, typename EntityListIterator, typename ComponentPools>
		typename component_iterators<
			EntityList, ComponentPools
		>::type	make_component_ends(EntityListIterator entity_iter, ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, end_pool<EntityListIterator>(entity_iter));
		}
		
		// --------------------------------------------------------------------
		//
		struct advance_component_iterator
		{
			advance_component_iterator()
			{}

		    template<typename T>
		    void operator()(T& t) const
		    {
				++t;
		    }
		};
	}

	// ------------------------------------------------------------------------
	//
	template<
			typename EntityList
		, 	typename ComponentPoolsTuple
		, 	typename ComponentPoolIterators
		>
	class entity_component_iterator 
		: public boost::iterator_facade<
			entity_component_iterator<EntityList, ComponentPoolsTuple, ComponentPoolIterators>
		,	ComponentPoolIterators
		,	boost::forward_traversal_tag
		,	ComponentPoolIterators
		>
	{
	public:	

		typedef typename EntityList::const_iterator entity_iterator; 

		entity_component_iterator(
			entity_iterator ent_begin,
			entity_iterator ent_end,
			ComponentPoolIterators&& iterator_tuple)
			: entity_iter_(std::move(ent_begin))
			, entity_end_(std::move(ent_end))
			, pool_iterators_(std::move(iterator_tuple))
		{}

	private:

		typedef boost::iterator_facade<
			entity_component_iterator<EntityList, ComponentPoolsTuple, ComponentPoolIterators>
		,	ComponentPoolIterators
		,	boost::forward_traversal_tag
		,	ComponentPoolIterators
		> base;

		friend class boost::iterator_core_access;

		void increment()
		{
			DAILY_AUTO_INSTRUMENT_NODE(entity_component_iterator__increment);
			++entity_iter_;
			if(entity_iter_ != entity_end_)
			{
				boost::fusion::for_each(
					pool_iterators_,
					detail::advance_component_iterator()
				);
			}
		}

		bool equal(entity_component_iterator const& other) const
		{
			return entity_iter_ == other.entity_iter_;
		}

		ComponentPoolIterators dereference() const
		{
			return pool_iterators_;
		}
		
		entity_iterator entity_iter_;
		entity_iterator entity_end_;
		ComponentPoolIterators pool_iterators_;
	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple,
		typename detail::component_iterators<EntityList, ComponentPoolsTuple>::type
	> begin(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_iterators<EntityList, ComponentPoolsTuple>::type
		>(begin(entities), end(entities), detail::make_component_begins<EntityList>(begin(entities), pools));
	}

	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple, 
		typename detail::component_iterators<EntityList, ComponentPoolsTuple>::type
	> end(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_iterators<EntityList, ComponentPoolsTuple>::type
		>(end(entities), end(entities), detail::make_component_ends<EntityList>(end(entities), pools));
	}
}

#endif // _ENTITY_ENTITYCOMPONENTITERATOR_H_INCLUDED_