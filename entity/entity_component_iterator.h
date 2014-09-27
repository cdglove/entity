//! \file entity/entity_component_iterator.h
//
// Represents an iterator that ties a component to an
// entity pool.  Allows us to iterate through an
// entity pool and component pool at the same time.
// 
#pragma once
#ifndef _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_
#define _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
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
			typedef typename T::type* type;
		};

		template<typename ComponentTypes>
		struct generate_value_type
		{
			typedef
			typename boost::fusion::result_of::as_vector<
				typename boost::mpl::push_front<
					typename boost::mpl::transform<
						ComponentTypes,
						extract_component_ptr_type<boost::mpl::_1>
					>::type,
					entity
				>::type
			>::type type;
		};

		template<typename T>
		struct iterator_pair
		{
			T current;
			T end;
		};

		template<typename T>
		struct extract_iterator_pair
		{
			typedef	iterator_pair<
				typename T::iterator
			> type;
		};

		template<typename ComponentTypes>
		struct component_ranges
		{
			typedef 
			typename boost::fusion::result_of::as_vector<
				typename boost::mpl::transform<
					ComponentTypes,
					extract_iterator_pair<boost::mpl::_1>
				>::type
			>::type type;
		};

		template<typename ComponentPool>
		typename component_ranges<boost::mpl::vector<ComponentPool>>::type
		make_component_begin_ranges(ComponentPool& pool)
		{
		 	typename component_ranges<
				boost::mpl::vector<ComponentPool>
			>::type ret_val;

			using boost::fusion::at_c;
			at_c<0>(ret_val).current = pool.begin();
			at_c<0>(ret_val).end = pool.end();
			return ret_val;
		}

		template<typename ComponentPool, typename ComponentPool1>
		typename component_ranges<boost::mpl::vector<ComponentPool, ComponentPool1>>::type
		make_component_begin_ranges(ComponentPool& pool, ComponentPool1& pool1)
		{
		 	typename component_ranges<
				boost::mpl::vector<ComponentPool, ComponentPool1>
			>::type ret_val;

			using boost::fusion::at_c;
			at_c<0>(ret_val).current = pool.begin();
			at_c<0>(ret_val).end = pool.end();
			at_c<1>(ret_val).current = pool1.begin();
			at_c<1>(ret_val).end = pool1.end();
			return ret_val;
		}

		template<typename ComponentPool>
		typename component_ranges<boost::mpl::vector<ComponentPool>>::type
		make_component_end_ranges(ComponentPool& pool)
		{
		 	typename component_ranges<
				boost::mpl::vector<ComponentPool>
			>::type ret_val;

			using boost::fusion::at_c;
			at_c<0>(ret_val).current = pool.end();
			at_c<0>(ret_val).end = pool.end();
			return ret_val;
		}

		template<typename ComponentPool, typename ComponentPool1>
		typename component_ranges<boost::mpl::vector<ComponentPool, ComponentPool1>>::type
		make_component_end_ranges(ComponentPool& pool, ComponentPool1& pool1)
		{
		 	typename component_ranges<
				boost::mpl::vector<ComponentPool, ComponentPool1>
			>::type ret_val;

			using boost::fusion::at_c;
			at_c<0>(ret_val).current = pool.end();
			at_c<0>(ret_val).end = pool.end();
			at_c<1>(ret_val).current = pool1.end();
			at_c<1>(ret_val).end = pool1.end();
			return ret_val;
		}

		struct advance_component_iterator
		{
			advance_component_iterator(entity e)
				: m_Entity(e)
			{}

		    template<typename T>
		    void operator()(T& t) const
		    {
				t.current.advance(m_Entity);
		    }

		    entity m_Entity;
		};

		template<int Idx, int Len>
		struct extract_component_ptrs;

		template<int Len>
		struct extract_component_ptrs<0, Len>
		{
			template<typename I, typename R>
			void apply(I const& iter, R& r) const
			{
				using boost::fusion::at_c;
				auto& i = at_c<Len-1>(iter);
				if(i.current.get_entity() == at_c<0>(r))
				{
					// Dont need to subtract one because index
					// 0 stored the entity.
					at_c<Len>(r) = &*i.current;
				}
			}
		};

		template<int Idx, int Len>
		struct extract_component_ptrs
		{
			template<typename I, typename R>
			void apply(I const& iter, R& r) const
			{
				using boost::fusion::at_c;
				auto& i = at_c<Len-Idx-1>(iter);
				if(i.current.get_entity() == at_c<0>(r))
				{
					// Dont need to subtract one because index
					// 0 stored the entity.
					at_c<Len-Idx>(r) = &*i.current;
					extract_component_ptrs<Idx-1, Len>().apply(iter, r);
				}
			}
		};
	}

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPools>
	class entity_component_iterator 
		: public boost::iterator_facade<
			entity_component_iterator<EntityList, ComponentPools>
		,	typename detail::generate_value_type<ComponentPools>::type
		,	boost::forward_traversal_tag
		,	typename detail::generate_value_type<ComponentPools>::type
		>
	{
	public:	

		typedef typename EntityList::const_iterator entity_iterator; 

		entity_component_iterator(
			entity_iterator ei,
			typename detail::component_ranges<ComponentPools>::type&& components)
			: m_EntityIter(std::move(ei))
			, m_Components(std::move(components))
		{}

	private:

		typedef boost::iterator_facade<
			entity_component_iterator<EntityList, ComponentPools>
		,	typename detail::generate_value_type<ComponentPools>::type
		,	boost::forward_traversal_tag
		,	typename detail::generate_value_type<ComponentPools>::type
		> base;

		friend class boost::iterator_core_access;

		void increment()
		{
			DAILY_AUTO_INSTRUMENT_NODE(entity_component_iterator__increment);
			++m_EntityIter;
			boost::fusion::for_each(
				m_Components,
				detail::advance_component_iterator(*m_EntityIter)
			);
		}

		bool equal(entity_component_iterator const& other) const
		{
			return m_EntityIter == other.m_EntityIter;
		}

		typename base::reference dereference() const
		{
			typename base::reference ret_val;

			using boost::fusion::at_c;
			at_c<0>(ret_val) = *m_EntityIter;
			detail::extract_component_ptrs<
				boost::mpl::size<ComponentPools>::value-1, 
				boost::mpl::size<ComponentPools>::value
			>().apply(m_Components, ret_val);

			return ret_val;
		}
		
		entity_iterator m_EntityIter;
		typename detail::component_ranges<ComponentPools>::type   m_Components;
	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPool>
	entity_component_iterator<
		EntityList, 
		boost::mpl::vector<ComponentPool>
	> begin(EntityList const& entities, ComponentPool& pool)
	{
		typedef boost::mpl::vector<ComponentPool> component_list;

		return entity_component_iterator<
			EntityList,
			component_list
		>(begin(entities), detail::make_component_begin_ranges(pool));
	}

	template<typename EntityList, typename ComponentPool>
	entity_component_iterator<
		EntityList, 
		boost::mpl::vector<ComponentPool>
	> end(EntityList const& entities, ComponentPool& pool)
	{
		typedef boost::mpl::vector<ComponentPool> component_list;

		return entity_component_iterator<
			EntityList,
			component_list
		>(end(entities), detail::make_component_end_ranges(pool));
	}

	template<typename EntityList, typename ComponentPool,  typename ComponentPool1>
	entity_component_iterator<
		EntityList, 
		boost::mpl::vector<ComponentPool, ComponentPool1>
	> begin(
		EntityList const& entities, 
		ComponentPool& pool,
		ComponentPool1& pool1)
	{
		typedef boost::mpl::vector<ComponentPool, ComponentPool1> component_list;
		
		return entity_component_iterator<
			EntityList,
			component_list
		>(begin(entities), detail::make_component_begin_ranges(pool, pool1));
	}

	template<typename EntityList, typename ComponentPool, typename ComponentPool1>
	entity_component_iterator<
		EntityList, 
		boost::mpl::vector<ComponentPool, ComponentPool1>
	> end(EntityList const& entities, ComponentPool& pool, ComponentPool1& pool1)
	{
		typedef boost::mpl::vector<ComponentPool, ComponentPool1> component_list;

		return entity_component_iterator<
			EntityList,
			component_list
		>(end(entities), detail::make_component_end_ranges(pool, pool1));
	}
}

#endif // _COMPONENT_ENTITYCOMPONENTITERATOR_H_INCLUDED_