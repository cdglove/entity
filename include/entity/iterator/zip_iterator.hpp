// ****************************************************************************
// entity/iterator/zip_iterator.hpp
//
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_ITERATOR_ZIPITERATOR_H_INCLUDED_
#define _ENTITY_ITERATOR_ZIPITERATOR_H_INCLUDED_

#include <tuple>
#include <utility>
#include <boost/iterator/iterator_facade.hpp>

#include "entity/config.hpp"  // IWYU pragma: keep
#include "entity/component/detail/get_helper.hpp"
#include "entity/type_traits/component_pool.hpp"

// ----------------------------------------------------------------------------
//
namespace entity { namespace iterator {

/// \brief Zip together a list of entities plus an arbitrary number of component
/// pools to allow iteration in parallel
///
template<typename EntityIterator, typename... ComponentPools>
class zip_iterator
	: public boost::iterator_facade<
	    zip_iterator<EntityIterator, ComponentPools...>
	  , std::tuple<typename type_traits::optional_type_of_pool<ComponentPools>::type...>
	  , boost::forward_traversal_tag
	  , std::tuple<typename type_traits::optional_type_of_pool<ComponentPools>::type...>
    >
{
public:

	zip_iterator(EntityIterator iter, ComponentPools&... pools)
		: entity_iterator_(iter)
		, pools_(std::make_tuple(component::detail::make_get_helper(pools)...))
	{}

private:

	friend class boost::iterator_core_access;

	void increment()
	{
		++entity_iterator_;
	}

	bool equal(zip_iterator const& other) const
	{
		return entity_iterator_ == other.entity_iterator_;
	}

	typedef std::tuple<
		typename type_traits::optional_type_of_pool<ComponentPools>::type...
	> reference_type;

	template<std::size_t... Indices>
	reference_type get_impl(std::index_sequence<Indices...>) const
	{
		entity e = *entity_iterator_;
		return std::make_tuple(std::get<Indices>(pools_).get(e)...);
	}

	reference_type dereference() const
	{
		return get_impl(std::make_index_sequence<sizeof...(ComponentPools)>());
	}

	EntityIterator entity_iterator_;

	std::tuple<
		component::detail::get_helper<ComponentPools>...
	> pools_;
};

// ----------------------------------------------------------------------------
//
template<typename EntityIterator, typename... ComponentPools>
zip_iterator<
	EntityIterator, ComponentPools...
> make_zip_iterator(EntityIterator iter, ComponentPools&... pools)
{
	return zip_iterator<EntityIterator, ComponentPools...>(iter, pools...);
}

} } // namespace entity { namespace iterator {

#endif // _ENTITY_ITERATOR_ZIPITERATOR_H_INCLUDED_