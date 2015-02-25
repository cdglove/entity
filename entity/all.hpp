// ****************************************************************************
// entity/all.hpp
//
// Master include file to bring in all of entity
// For lazy programmers.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_ALL_H_INCLUDED_
#define _ENTITY_ALL_H_INCLUDED_

#include <entity/component_creation_queue.hpp>
#include <entity/component_destruction_queue.hpp>
#include <entity/dense_component_pool.hpp>
#include <entity/entity.hpp>
#include <entity/entity_index.hpp>
#include <entity/entity_pool.hpp>
#include <entity/entity_range.hpp>
#include <entity/saturated_component_pool.hpp>
#include <entity/sparse_component_pool.hpp>
#include <entity/algorithm/for_each.hpp>
#include <entity/component/tie.hpp>
#include <entity/component/zip.hpp>

#endif // _ENTITY_ALL_H_INCLUDED_