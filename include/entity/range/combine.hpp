// ****************************************************************************
// entity/range/combine.hpp
//
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef ENTITY_RANGE_COMBINE_H_INCLUDED_
#define ENTITY_RANGE_COMBINE_H_INCLUDED_

#include "entity/config.hpp"  // IWYU pragma: keep

#include <boost/range/iterator_range_core.hpp>
#include <boost/range/combine.hpp>
#include "entity/iterator/zip_iterator.hpp"

// ----------------------------------------------------------------------------
//
namespace entity { namespace range {

// ----------------------------------------------------------------------------
//
template<typename EntityRange, typename... ComponentPool>
boost::iterator_range<
	iterator::zip_iterator<
		typename EntityRange::iterator, 
		ComponentPool...
	>
> combine(EntityRange& entities, ComponentPool&... pools)
{
	return boost::make_iterator_range(
		iterator::make_zip_iterator(entities.begin(), pools...),
		iterator::make_zip_iterator(entities.end(), pools...)
	);
}

// ----------------------------------------------------------------------------
//
template<typename ComponentPool>
boost::iterator_range<
	typename ComponentPool::optional_iterator
> make_optional_range(ComponentPool& pool)
{
	return boost::make_iterator_range(pool.optional_begin(), pool.optional_end());
}

// ----------------------------------------------------------------------------
//
template<typename... ComponentPool>
auto combine_optional(ComponentPool&... pools)
{
	return boost::range::combine(
		make_optional_range(pools)...
	);
}

} } // namespace entity { namespace range {

#endif // ENTITY_RANGE_COMBINE_H_INCLUDED_