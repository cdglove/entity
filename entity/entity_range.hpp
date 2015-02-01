//! \file entity/entity_iterator.h
//
// Iterates over a list of entities and a zipped component
// pool as a tuple.
// 
#pragma once
#ifndef _ENTITY_ENTITYITERATOR_H_INCLUDED_
#define _ENTITY_ENTITYITERATOR_H_INCLUDED_

#include <boost/iterator/iterator_facade.hpp>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"
#include "entity/traits/iterator_traits.hpp"

namespace boost {
namespace iterators {
struct forward_traversal_tag;
}  // namespace iterators
}  // namespace boost

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
		,	ZippedComponentWindows
		,	boost::forward_traversal_tag
		,	ZippedComponentWindows
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

		ZippedComponentWindows dereference() const
		{
			return windows_;
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
		,	ZippedComponentWindows
		,	boost::forward_traversal_tag
		,	ZippedComponentWindows
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

		ZippedComponentWindows dereference() const
		{
			return windows_;
		}

		EntityListIterator entity_iter_;
		EntityListIterator end_;
		ZippedComponentWindows windows_;

	};

	// ------------------------------------------------------------------------
	//
	template<typename Iterator, typename EndIterator = Iterator>
	class entity_range
	{
	public:

		entity_range(Iterator begin, EndIterator end)
			: begin_(std::move(begin))
			, end_(std::move(end))
		{}

		Iterator begin()
		{
			return begin_;
		}

		EndIterator end()
		{
			return end_;
		}

	private:

		Iterator begin_;
		EndIterator end_;
	};

	template<typename Iterator, typename EndIterator>
	Iterator begin(entity_range<Iterator, EndIterator>& r)
	{
		return r.begin();
	}

	template<typename Iterator, typename EndIterator>
	EndIterator end(entity_range<Iterator, EndIterator>& r)
	{
		return r.end();
	}

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_begin(iterator_traits::is_skipping_tag, EntityList const& entities, ZippedComponentWindows windows) -> entity_skipping_iterator<
		decltype(begin(entities)),
		ZippedComponentWindows>
	{
		return entity_skipping_iterator<
			decltype(begin(entities)),
			ZippedComponentWindows
		>(begin(entities), end(entities), std::move(windows));
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_end(iterator_traits::is_skipping_tag, EntityList const& entities, ZippedComponentWindows windows) -> entity_skipping_iterator<
		decltype(end(entities)),
		ZippedComponentWindows>
	{
		return entity_skipping_iterator<
			decltype(begin(entities)),
			ZippedComponentWindows
		>(end(entities), end(entities), std::move(windows));
	}

	// --------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_begin(iterator_traits::is_incremental_tag, EntityList const& entities, ZippedComponentWindows windows) -> entity_incremental_iterator<
		decltype(entities.begin()),
		ZippedComponentWindows>
	{
		return entity_incremental_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.begin(), entities.end(), std::move(windows));
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_end(iterator_traits::is_incremental_tag, EntityList& entities, ZippedComponentWindows windows) -> entity_incremental_iterator<
		decltype(entities.end()),
		ZippedComponentWindows>
	{
		return entity_incremental_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.end(), entities.end(), std::move(windows));
	}

	// --------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_begin(EntityList const& list, ZippedComponentWindows windows) -> decltype(make_entity_begin(iterator_traits::entity_list_is_incremental<EntityList>(), list, windows))
	{
		return make_entity_begin(
			iterator_traits::entity_list_is_incremental<EntityList>(),
			list, 
			std::move(windows)
		);
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_end(EntityList const& list, ZippedComponentWindows windows) -> decltype(make_entity_end(iterator_traits::entity_list_is_incremental<EntityList>(), list, windows))
	{
		return make_entity_end(
			iterator_traits::entity_list_is_incremental<EntityList>(),
			list,
			std::move(windows)
		);
	}

	// --------------------------------------------------------------------
	//
	template<typename EntityList, typename ZippedComponentWindows>
	auto make_entity_range(EntityList const& list, ZippedComponentWindows windows) -> entity_range<decltype(make_entity_begin(list, windows)), decltype(make_entity_end(list, windows))>
	{
		return entity_range<
			decltype(make_entity_begin(list, windows)), 
			decltype(make_entity_end(list, windows))>(
				make_entity_begin(list, windows),
				make_entity_end(list, windows)
			)
		;
	}
}

#endif // _ENTITY_ENTITYITERATOR_H_INCLUDED_
