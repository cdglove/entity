//! \file entity/functional/is_valid_component.h
//
// Returns true if the given component ptr is valid.
// 
#pragma once
#ifndef _ENTITY_FUNCTIONAL_ISVALIDCOMPONENT_H_INCLUDED_
#define _ENTITY_FUNCTIONAL_ISVALIDCOMPONENT_H_INCLUDED_

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	struct is_valid_component
	{
		template<typename ComponentIterator>
		bool operator()(ComponentIterator c)
		{
			return c.is_valid();
		}
	};
}
#endif // _ENTITY_ALGORITHM_ISVALIDCOMPONENT_H_INCLUDED_