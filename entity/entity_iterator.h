//! \file entity/entity_iterator.h
//
// Iterates over a list of entities and a zipped component
// pool as a tuple.
// 
#pragma once
#ifndef _ENTITY_ENTITYITERATOR_H_INCLUDED_
#define _ENTITY_ENTITYITERATOR_H_INCLUDED_

#include <boost/iterator/iterator_facade.hpp>

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename EntityListIterator, typename ZippedComponentWindows>
	class packed_entity_iterator
		: public boost::iterator_facade<
		    packed_entity_iterator<EntityListIterator, ZippedComponentWindows>
		,	typename ZippedComponentWindows::windows_type
		,	boost::forward_traversal_tag
		,	typename ZippedComponentWindows::windows_type
		>
	{
	public:

		packed_entity_iterator(EntityListIterator begin, EntityListIterator end, ZippedComponentWindows windows)
			: entity_iter_(begin)
			, end_(end)
			, windows_(windows)
		{}

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

		bool equal(packed_entity_iterator const& other) const
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

	template<typename EntityListIterator, typename ZippedComponentWindows>
	class unpacked_entity_iterator
		: public boost::iterator_facade<
		    unpacked_entity_iterator<EntityListIterator, ZippedComponentWindows>
		,	typename ZippedComponentWindows::windows_type
		,	boost::forward_traversal_tag
		,	typename ZippedComponentWindows::windows_type
		>
	{
	public:

		unpacked_entity_iterator(EntityListIterator begin, EntityListIterator end, ZippedComponentWindows windows)
			: entity_iter_(begin)
			, end_(end)
			, windows_(windows)
		{}

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

		bool equal(unpacked_entity_iterator const& other) const
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

	template<typename EntityList, typename ZippedComponentWindows>
	auto begin(EntityList const& entities, ZippedComponentWindows windows) -> unpacked_entity_iterator<
																			decltype(begin(entities)),
																			ZippedComponentWindows>
	{
		return unpacked_entity_iterator<
			      decltype(begin(entities)),
				  ZippedComponentWindows
				>(begin(entities), end(entities), windows)
		;
	}

	template<typename EntityList, typename ZippedComponentWindows>
	auto end(EntityList const& entities, ZippedComponentWindows windows) -> unpacked_entity_iterator<
																		  decltype(end(entities)),
																		  ZippedComponentWindows>
	{
		return unpacked_entity_iterator<
			       decltype(begin(entities)),
				   ZippedComponentWindows
		       >(end(entities), end(entities), windows)
		;
	}

	// ------------------------------------------------------------------------
	//
	template<typename ZippedComponentWindows>
	auto begin(entity_pool const& entities, ZippedComponentWindows windows) -> packed_entity_iterator<
		decltype(entities.begin()),
		ZippedComponentWindows>
	{
		return packed_entity_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.begin(), entities.end(), windows)
		;
	}

	template<typename ZippedComponentWindows>
	auto end(entity_pool const& entities, ZippedComponentWindows windows) -> packed_entity_iterator<
		decltype(entities.end()),
		ZippedComponentWindows>
	{
		return packed_entity_iterator<
			decltype(entities.begin()),
			ZippedComponentWindows
		>(entities.end(), entities.end(), windows)
		;
	}

}

#endif // _ENTITY_ENTITYITERATOR_H_INCLUDED_