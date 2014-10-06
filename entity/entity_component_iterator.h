//! \file entity/entity_component_iterator.h
//
// Represents an iterator that ties a component to an
// entity pool.  Allows us to iterate through an
// entity pool and component pool at the same time.
// 
#pragma once
#ifndef _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_
#define _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_

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
		struct extract_component_ptr_type
		{
			typedef typename std::decay<T>::type::type* type;
		};

		template<typename ComponentPoolTuple>
		struct generate_value_type
		{
			typedef
			typename boost::fusion::result_of::as_vector<
				typename boost::mpl::push_front<
					typename boost::mpl::transform<
						ComponentPoolTuple,
						extract_component_ptr_type<boost::mpl::_1>
					>::type,
					entity
				>::type
			>::type type;
		};

		// template<typename T>
		// struct iterator_pair
		// {
		// 	T current;
		// 	T end;
		// };

		// template<typename T>
		// struct extract_iterator_pair
		// {
		// 	typedef	iterator_pair<
		// 		typename T::iterator
		// 	> type;
		// };

		// template<typename ComponentTypes>
		// struct component_ranges
		// {
		// 	typedef 
		// 	typename boost::fusion::result_of::as_vector<
		// 		typename boost::mpl::transform<
		// 			ComponentTypes,
		// 			extract_iterator_pair<boost::mpl::_1>
		// 		>::type
		// 	>::type type;
		// };

		// template<typename ComponentPool>
		// typename component_ranges<boost::mpl::vector<ComponentPool>>::type
		// make_component_begin_ranges(ComponentPool& pool)
		// {
		//  	typename component_ranges<
		// 		boost::mpl::vector<ComponentPool>
		// 	>::type ret_val;

		// 	using boost::fusion::at_c;
		// 	at_c<0>(ret_val).current = pool.begin();
		// 	at_c<0>(ret_val).end = pool.end();
		// 	return ret_val;
		// }

		// template<typename ComponentPool, typename ComponentPool1>
		// typename component_ranges<boost::mpl::vector<ComponentPool, ComponentPool1>>::type
		// make_component_begin_ranges(ComponentPool& pool, ComponentPool1& pool1)
		// {
		//  	typename component_ranges<
		// 		boost::mpl::vector<ComponentPool, ComponentPool1>
		// 	>::type ret_val;

		// 	using boost::fusion::at_c;
		// 	at_c<0>(ret_val).current = pool.begin();
		// 	at_c<0>(ret_val).end = pool.end();
		// 	at_c<1>(ret_val).current = pool1.begin();
		// 	at_c<1>(ret_val).end = pool1.end();
		// 	return ret_val;
		// }

		// template<typename ComponentPool>
		// typename component_ranges<boost::mpl::vector<ComponentPool>>::type
		// make_component_end_ranges(ComponentPool& pool)
		// {
		//  	typename component_ranges<
		// 		boost::mpl::vector<ComponentPool>
		// 	>::type ret_val;

		// 	using boost::fusion::at_c;
		// 	at_c<0>(ret_val).current = pool.end();
		// 	at_c<0>(ret_val).end = pool.end();
		// 	return ret_val;
		// }

		// template<typename ComponentPool, typename ComponentPool1>
		// typename component_ranges<boost::mpl::vector<ComponentPool, ComponentPool1>>::type
		// make_component_end_ranges(ComponentPool& pool, ComponentPool1& pool1)
		// {
		//  	typename component_ranges<
		// 		boost::mpl::vector<ComponentPool, ComponentPool1>
		// 	>::type ret_val;

		// 	using boost::fusion::at_c;
		// 	at_c<0>(ret_val).current = pool.end();
		// 	at_c<0>(ret_val).end = pool.end();
		// 	at_c<1>(ret_val).current = pool1.end();
		// 	at_c<1>(ret_val).end = pool1.end();
		// 	return ret_val;
		// }

		template<typename ComponentPoolsTuple>
		struct component_iterators
		{
			typedef typename boost::mpl::result_of::transform<
						ComponentPoolsTuple const, begin_pool
					>::type type;
		};

		// --------------------------------------------------------------------
		//
		struct begin_pool
		{
			template<typename Pool>
			typename Pool::iterator operator()(Pool& p)
			{
				return p.begin();
			}
		};

		template<typename ComponentPoolsTuple>
		struct component_begin_iterators
		{
			typedef typename boost::fusion::result_of::transform<
						ComponentPoolsTuple const, begin_pool
					>::type type;
		};

		struct end_pool
		{
			template<typename Pool>
			typename Pool::iterator operator()(Pool& p)
			{
				return p.end();
			}
		};

		template<typename ComponentPoolsTuple>
		struct component_end_iterators
		{
			typedef typename boost::fusion::result_of::transform<
						ComponentPoolsTuple const, end_pool
					>::type type;
		};

		template<typename ComponentPools>
		typename component_begin_iterators<
			ComponentPools
		>::type	make_component_begins(ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, begin_pool());
		}

		template<typename ComponentPools>
		typename component_end_iterators<
			ComponentPools
		>::type	make_component_ends(ComponentPools const& pools)
		{
			return boost::fusion::transform(pools, end_pool());
		}
		
		// --------------------------------------------------------------------
		//
		struct advance_component_iterator
		{
			advance_component_iterator(entity e)
				: m_Entity(e)
			{}

		    template<typename T>
		    void operator()(T& t) const
		    {
				t.advance(m_Entity);
		    }

		    entity m_Entity;
		};

		// --------------------------------------------------------------------
		//
		struct extract_value_ptr
		{
			extract_value_ptr(entity e)
				: m_Entity(e)
			{}

			template<typename Iterator>
			typename Iterator::value_type* operator()(Iterator& iterator)
			{
				if(iterator.get_entity() == m_Entity)
				{
					return &*iterator;
				}

				return nullptr;
			}

			entity m_Entity;
		};
		// template<int Idx, int Len>
		// struct extract_component_ptrs;

		// template<int Len>
		// struct extract_component_ptrs<0, Len>
		// {
		// 	template<typename I, typename R>
		// 	void apply(I const& iter, R& r) const
		// 	{
		// 		using boost::fusion::at_c;
		// 		auto& i = at_c<Len-1>(iter);
		// 		if(i.current.get_entity() == at_c<0>(r))
		// 		{
		// 			// Dont need to subtract one because index
		// 			// 0 stored the entity.
		// 			at_c<Len>(r) = &*i.current;
		// 		}
		// 	}
		// };

		// template<int Idx, int Len>
		// struct extract_component_ptrs
		// {
		// 	template<typename I, typename R>
		// 	void apply(I const& iter, R& r) const
		// 	{
		// 		using boost::fusion::at_c;
		// 		auto& i = at_c<Len-Idx-1>(iter);
		// 		if(i.current.get_entity() == at_c<0>(r))
		// 		{
		// 			// Dont need to subtract one because index
		// 			// 0 stored the entity.
		// 			at_c<Len-Idx>(r) = &*i.current;
		// 			extract_component_ptrs<Idx-1, Len>().apply(iter, r);
		// 		}
		// 	}
		// };
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
		,	typename detail::generate_value_type<ComponentPoolsTuple>::type
		,	boost::forward_traversal_tag
		,	typename detail::generate_value_type<ComponentPoolsTuple>::type
		>
	{
	public:	

		typedef typename EntityList::const_iterator entity_iterator; 

		entity_component_iterator(
			entity_iterator ei,
			ComponentPoolIterators&& iterator_tuple)
			: m_EntityIter(std::move(ei))
			, m_PoolIterators(std::move(iterator_tuple))
		{}

	private:

		typedef boost::iterator_facade<
			entity_component_iterator<EntityList, ComponentPoolsTuple, ComponentPoolIterators>
		,	typename detail::generate_value_type<ComponentPoolsTuple>::type
		,	boost::forward_traversal_tag
		,	typename detail::generate_value_type<ComponentPoolsTuple>::type
		> base;

		friend class boost::iterator_core_access;

		void increment()
		{
			DAILY_AUTO_INSTRUMENT_NODE(entity_component_iterator__increment);
			++m_EntityIter;
			boost::fusion::for_each(
				m_PoolIterators,
				detail::advance_component_iterator(*m_EntityIter)
			);
		}

		bool equal(entity_component_iterator const& other) const
		{
			return m_EntityIter == other.m_EntityIter;
		}

		typename base::reference dereference() const
		{
			//typename base::reference ret_val;

			//using boost::fusion::at_c;
			//at_c<0>(ret_val) = *m_EntityIter;

			return boost::fusion::transform(
				m_PoolIterators,
				detail::extract_value_ptr(*m_EntityIter)
			);

			
			// detail::extract_component_ptrs<
			// 	boost::mpl::size<ComponentPoolIterators>::value-1, 
			// 	boost::mpl::size<ComponentPoolIterators>::value
			// >().apply(m_Components, ret_val);

			// return ret_val;
		}
		
		entity_iterator m_EntityIter;
		ComponentPoolIterators m_PoolIterators;
	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple,
		typename detail::component_begin_iterators<ComponentPoolsTuple>::type
	> begin(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_begin_iterators<ComponentPoolsTuple>::type
		>(begin(entities), detail::make_component_begins(pools));
	}

	template<typename EntityList, typename ComponentPoolsTuple>
	entity_component_iterator<
		EntityList, 
		ComponentPoolsTuple, 
		typename detail::component_end_iterators<ComponentPoolsTuple>::type
	> end(EntityList const& entities, ComponentPoolsTuple const& pools)
	{
		return entity_component_iterator<
			EntityList,
			ComponentPoolsTuple, 
			typename detail::component_end_iterators<ComponentPoolsTuple>::type
		>(end(entities), detail::make_component_ends(pools));
	}

	template<typename Pool>
	boost::fusion::vector<Pool&> tie(Pool& pool)
	{
		return boost::fusion::vector<Pool&>(pool);
	}

	template<typename Pool1, typename Pool2>
	boost::fusion::vector<Pool1&, Pool2&> tie(Pool1& pool_1, Pool2& pool_2)
	{
		return boost::fusion::vector<Pool1&, Pool2&>(pool_1, pool_2);
	}

	template<typename Pool1, typename Pool2, typename Pool3>
	boost::fusion::vector<Pool1&, Pool2&, Pool3&> tie(Pool1& pool_1, Pool2& pool_2, Pool3& pool_3)
	{
		return boost::fusion::vector<Pool1&, Pool2&, Pool3&>(pool_1, pool_2, pool_3);
	}

	// template<typename EntityList, typename... ComponentPools>
	// entity_component_iterator<
	// 	EntityList, 
	// 	typename detail::component_begin_iterators<ComponentPoolsTuple>::type
	// > begin(EntityList const& entities, ComponentPools...&& pools)
	// {
	// 	return entity_component_iterator<
	// 		EntityList,
	// 		typename detail::component_begin_iterators<ComponentPoolsTuple>::type
	// 	>(begin(entities), detail::make_component_begins(pools));
	// }

	// template<typename EntityList, typename... ComponentPools>
	// entity_component_iterator<
	// 	EntityList, 
	// 	typename detail::component_end_iterators<ComponentPoolsTuple>::type
	// > end(EntityList const& entities, ComponentPoolsTuple const& pools)
	// {
	// 	return entity_component_iterator<
	// 		EntityList,
	// 		typename detail::component_end_iterators<ComponentPoolsTuple>::type
	// 	>(end(entities), detail::make_component_ends(pools));
	// }

	// template<typename EntityList, typename ComponentPool,  typename ComponentPool1>
	// entity_component_iterator<
	// 	EntityList, 
	// 	boost::mpl::vector<ComponentPool, ComponentPool1>
	// > begin(
	// 	EntityList const& entities, 
	// 	ComponentPool& pool,
	// 	ComponentPool1& pool1)
	// {
	// 	typedef boost::mpl::vector<ComponentPool, ComponentPool1> component_list;
		
	// 	return entity_component_iterator<
	// 		EntityList,
	// 		component_list
	// 	>(begin(entities), detail::make_component_begin_ranges(pool, pool1));
	// }

	// template<typename EntityList, typename ComponentPool, typename ComponentPool1>
	// entity_component_iterator<
	// 	EntityList, 
	// 	boost::mpl::vector<ComponentPool, ComponentPool1>
	// > end(EntityList const& entities, ComponentPool& pool, ComponentPool1& pool1)
	// {
	// 	typedef boost::mpl::vector<ComponentPool, ComponentPool1> component_list;

	// 	return entity_component_iterator<
	// 		EntityList,
	// 		component_list
	// 	>(end(entities), detail::make_component_end_ranges(pool, pool1));
	// }
}

#endif // _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_