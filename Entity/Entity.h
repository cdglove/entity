//! \file Entity/Entity.h
//
// Represents an entity which is made up of components.
// Essentially an ID but not incrementable, etc.
// 
#pragma once
#ifndef _COMPONENT_ENTITY_H_INCLUDED_
#define _COMPONENT_ENTITY_H_INCLUDED_

#include <boost/strong_typedef.hpp>

// ----------------------------------------------------------------------------
//
namespace Entity
{
	BOOST_STRONG_TYPEDEF(unsigned int, Entity);
}

#endif // _COMPONENT_ENTITY_H_INCLUDED_