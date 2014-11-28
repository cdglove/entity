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

// ----------------------------------------------------------------------------
//
namespace entity 
{
	namespace detail
	{
		template<typename T>
		struct extract_value_ptr_type
		{
			typedef typename std::decay<T>::type::type* type;
		};

		template<typename ComponentPoolTuple>
		struct generate_value_ptr_type
		{
			typedef
			typename boost::fusion::result_of::as_vector<
				typename boost::mpl::transform<
					ComponentPoolTuple,
					extract_value_ptr_type<boost::mpl::_1>
				>::type
			>::type type;
		};

		template<typename ComponentPool>
		struct extract_iterator
		{
			typedef typename 
			std::decay<
				ComponentPool
			>::type::iterator type;
		};

		template<typename ComponentPoolsTuple>
		struct component_iterators
		{
			typedef typename
			boost::mpl::transform<
				ComponentPoolsTuple, extract_iterator<boost::mpl::_1>
			>::type type;
		};

		// --------------------------------------------------------------------
		//
		struct begin_pool
		{
			template<typename Pool>
			typename Pool::iterator operator()(Pool& p) const
			{
				return p.begin();
			}
		};

		struct end_pool
		{
			template<typename Pool>
			typename Pool::iterator operator()(Pool& p) const
			{
				return p.end();
			}
		};

		template<typename ComponentPools>
		typename component_iterators<
			ComponentPools
		>::type	make_component_begins(ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, begin_pool());
		}

		template<typename ComponentPools>
		typename component_iterators<
			ComponentPools
		>::type	make_component_ends(ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, end_pool());
		}
		
		// --------------------------------------------------------------------
		//
		struct advance_component_iterator
		{
			advance_component_iterator(entity target)
				: target_(target)
			{}

		    template<typename T>
		    void operator()(T& t) const
		    {
				t.advance_to_target_entity(target_);
		    }

		    entity target_;
		};

		// --------------------------------------------------------------------
		//
		struct extract_value_ptr
		{
			extract_value_ptr(entity e)
				: entity_(e)
			{}

			template<typename Iterator>
			typename Iterator::pointer operator()(Iterator& i) const
			{
				return i.maybe_extract_ptr(entity_);
			}

			entity entity_;
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
			entity_iterator ei,
			ComponentPoolIterators&& iterator_tuple)
			: entity_iter_(std::move(ei))
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
			boost::fusion::for_each(
				pool_iterators_,
				detail::advance_component_iterator(*entity_iter_)
			);
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
		ComponentPoolIterators pool_iterators_;
	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple,
		typename detail::component_iterators<ComponentPoolsTuple>::type
	> begin(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_iterators<ComponentPoolsTuple>::type
		>(begin(entities), detail::make_component_begins(pools));
	}

	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple, 
		typename detail::component_iterators<ComponentPoolsTuple>::type
	> end(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_iterators<ComponentPoolsTuple>::type
		>(end(entities), detail::make_component_ends(pools));
	}
}

#endif // _ENTITY_ENTITYCOMPONENTITERATOR_H_INCLUDED_