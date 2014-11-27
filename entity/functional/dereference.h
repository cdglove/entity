//! \file entity/functional/is_valid_component.h
//
// Returns true if the given component ptr is valid.
// 
#pragma once
#ifndef _ENTITY_FUNCTIONAL_TOREFERENCESEQUENCE_H_INCLUDED_
#define _ENTITY_FUNCTIONAL_TOREFERENCESEQUENCE_H_INCLUDED_

namespace entity {

	struct dereference
	{
		template<typename ComponentIterator>
		typename ComponentIterator::reference operator()(ComponentIterator c) const
		{
			return *c;
		}
	};
}
#endif // _ENTITY_FUNCTIONAL_TOREFERENCESEQUENCE_H_INCLUDED_