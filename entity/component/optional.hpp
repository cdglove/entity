// ****************************************************************************
// entity/component/optional.hpp
//
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_COMPONENT_OPTIONAL_H_INCLUDED_
#define _ENTITY_COMPONENT_OPTIONAL_H_INCLUDED_

#include <boost/none.hpp>
#include <boost/utility/explicit_operator_bool.hpp>

// ----------------------------------------------------------------------------
//
namespace entity { namespace component {

/// \brief Represents an optional component entry from pools that may or may
/// not contain the specificed component. Similar to boost::optional but with
/// a more compact representation.
///
template<typename T>
class optional
{
public:

	// For boost iterator adapaters.
	typedef T element_type;

	optional(boost::none_t)
		: value_(nullptr)
	{}

	optional(T& value)
		: value_(&value)
	{}

	T& get()
	{
		return *value_;
	}

	T const& get() const
	{
		return *value_;
	}

	T& operator*()
	{
		return *value_;
	}

	T const& operator*() const
	{
		return *value_;
	}

	T* operator->()
	{
		return value_;
	}

	T const* operator->() const
	{
		return value_;
	}

	bool operator!() const
	{
		return value_ ? false : true;
	}

	BOOST_EXPLICIT_OPERATOR_BOOL();

private:

	T* value_;
};

} }

#endif // _ENTITY_COMPONENT_OPTIONAL_H_INCLUDED_