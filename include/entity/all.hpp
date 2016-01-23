// ****************************************************************************
// entity/all.hpp
//
// Master include file to bring in all of entity
// For lazy programmers.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_ALL_H_INCLUDED_
#define _ENTITY_ALL_H_INCLUDED_

#include <entity/entity.hpp>
#include <entity/entity_index.hpp>
#include <entity/entity_pool.hpp>
#include <entity/component/creation_queue.hpp>
#include <entity/component/destruction_queue.hpp>
#include <entity/component/dense_pool.hpp>
#include <entity/component/saturated_pool.hpp>
#include <entity/component/sparse_pool.hpp>
#include <entity/iterator/zip_iterator.hpp>
#include <entity/range/combine.hpp>

#endif // _ENTITY_ALL_H_INCLUDED_