// ****************************************************************************
// entity/component/required.hpp
//
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_COMPONENT_REQUIRED_H_INCLUDED_
#define _ENTITY_COMPONENT_REQUIRED_H_INCLUDED_

#include "entity/component/optional.hpp"
#include <boost/utility/explicit_operator_bool.hpp>

// ----------------------------------------------------------------------------
//
namespace entity { namespace component {

/// \brief Represents a required component, but contains the same interface as
/// optional. Is always valid.
///
template<typename T>
class required
{
public:

	// For boost iterator adapaters.
	typedef T element_type;

	required(T& value)
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

	operator optional<T>() const
	{
		return *value_;
	}

	BOOST_CONSTEXPR BOOST_FORCEINLINE bool operator!() const
	{
		return false;
	}

	BOOST_CONSTEXPR BOOST_EXPLICIT_OPERATOR_BOOL();

private:

	T* value_;
};

} }

#endif //  