//! \file entity/entity_iterator.h
//
// Iterates over a list of entities and a zipped component
// pool as a tuple.
// 
#pragma once
#ifndef _ENTITY_ENTITYITERATOR_H_INCLUDED_
#define _ENTITY_ENTITYITERATOR_H_INCLUDED_

#include <boost/iterator/iterator_facade.hpp>
#include <type_traits>
#include "entity/traits/iterator_traits.h"

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename EntityListIterator, typename ZippedComponentWindows>
	class entity_incremental_iterator
		: public boost::iterator_facade<
		    entity_incremental_iterator<EntityListIterator, ZippedComponentWindows>
		,	typename ZippedComponentWindows::windows_type
		,	boost::forward_traversal_tag
		,	typename ZippedComponentWindows::windows_type
		>
	{
	public:

		entity_incremental_iterator(EntityListIterator begin, EntityListIterator end, ZippedComponentWindows windows)
			: entity_iter_(begin)
			, end_(end)
			, windows_(windows)
		{}

		entity get_entity() const
		{
			return *entity_iter_;
		}

	private:

		friend class boost::iterator_core_access;

		void increment()
		{
			++entity_iter_;
			if(entity_iter_ != end_)
			{
				windows_.increment(*entity_iter_);
			}
		}

		bool equal(entity_incremental_iterator const& other) const
		{
			return entity_iter_ == other.entity_iter_;
		}

		typename ZippedComponentWindows::windows_type dereference() const
		{
			return windows_.get();
		}

		EntityListIterator entity_iter_;
		EntityListIterator end_;
		ZippedComponentWindows windows_;

	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityListIterator, typename ZippedComponentWindows>
	class entity_skipping_iterator
		: public boost::iterator_facade<
		    entity_skipping_iterator<EntityListIterator, ZippedComponentWindows>
		,	typename ZippedComponentWindows::windows_type
		,	boost::forward_traversal_tag
		,	typename ZippedComponentWindows::windows_type
		>
	{
	public:

		entity_skipping_iterator(EntityListIterator begin, EntityListIterator end, ZippedComponentWindows windows)
			: entity_iter_(begin)
			, end_(end)
			, windows_(windows)
		{
			if(entity_iter_ != end_)
			{
				windows_.advance(*entity_iter_);
			}
		}
		
		entity get_entity() const
		{
			return *entity_iter_;
		}

	private:

		friend class boost::iterator_core_access;

		void increment()
		{
			++entity_iter_;
			if(entity_iter_ != end_)
			{
				windows_.advance(*entity_iter_);
			}
		}

		bool equal(entity_skipping_iterator const& other) const
		{
			return entity_iter_ == other.entity_iter_;
		}

		typename ZippedComponentWindows::windows_type dereference() const
		{
			return windows_.get();
		}

		EntityListIterator entity_iter_;
		EntityListIterator end_;
		ZippedComponentWindows windows_;

	};

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto begin_entities(EntityList const& entities, ZippedComponentWindows windows, iterator_traits::is_skipping_tag) -> entity_skipping_iterator<
		decltype(begin(entities)),
		ZippedComponentWindows>
	{
		return entity_skipping_iterator<
			decltype(begin(entities)),
			ZippedComponentWindows
		>(begin(entities), end(entities), windows);
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto end_entities(EntityList const& entities, ZippedComponentWindows windows, iterator_traits::is_skipping_tag) -> entity_skipping_iterator<
		decltype(end(entities)),
		ZippedComponentWindows>
	{
		return entity_skipping_iterator<
			decltype(begin(entities)),
			ZippedComponentWindows
		>(end(entities), end(entities), windows);
	}

	// --------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto begin_entities(EntityList const& entities, ZippedComponentWindows windows, iterator_traits::is_incremental_tag) -> entity_incremental_iterator<
		decltype(entities.begin()),
		ZippedComponentWindows>
	{
		return entity_incremental_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.begin(), entities.end(), windows);
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto end_entities(EntityList& entities, ZippedComponentWindows windows, iterator_traits::is_incremental_tag) -> entity_incremental_iterator<
		decltype(entities.end()),
		ZippedComponentWindows>
	{
		return entity_incremental_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.end(), entities.end(), windows);
	}

	// --------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto begin_entities(EntityList const& list, ZippedComponentWindows windows) -> decltype(begin_entities(list, windows, iterator_traits::entity_list_is_incremental<EntityList>()))
	{
		return begin_entities(list, windows, iterator_traits::entity_list_is_incremental<EntityList>());
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto end_entities(EntityList const& list, ZippedComponentWindows windows) -> decltype(end_entities(list, windows, iterator_traits::entity_list_is_incremental<EntityList>()))
	{
		return end_entities(list, windows, iterator_traits::entity_list_is_incremental<EntityList>());
	}

	iterator_traits::is_incremental_tag incremental;
	iterator_traits::is_skipping_tag skipping;
}

#endif // _ENTITY_ENTITYITERATOR_H_INCLUDED_