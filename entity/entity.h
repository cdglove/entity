//! \file entity/entity.h
//
// Represents an entity which is made up of components.
// Essentially an ID but not incrementable, etc.
// 
#pragma once
#ifndef _ENTITY_ENTITY_H_INCLUDED_
#define _ENTITY_ENTITY_H_INCLUDED_

#include "entity/config.h"
#include <boost/strong_typedef.hpp>

// ----------------------------------------------------------------------------
//
namespace entity
{
	BOOST_STRONG_TYPEDEF(unsigned int, entity);
}

#endif // _ENTITY_ENTITY_H_INCLUDED_
