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
		template<typename Component>
		Component& operator()(Component* c) const
		{
			return *c;
		}

		//template<typename Component>
		//Component const& operator()(Component const* c) const
		//{
		//	return *c;
		//}
	};
}
#endif // _ENTITY_FUNCTIONAL_TOREFERENCESEQUENCE_H_INCLUDED_