// ****************************************************************************
// test/performance_common.h
//
// Part of the test harness for entity.
// 
// Copyright Chris Glover 2014
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once

#ifndef TEST_SIZE
#  ifdef _DEBUG
#    define TEST_SIZE (512)
#  endif
#endif

#ifndef TEST_SIZE
#  if DAILY_ENABLE_INSTRUMENTATION
#    define TEST_SIZE (1024 * 256)
#  endif
#endif

#ifndef TEST_SIZE
#  define TEST_SIZE (1024*2048)
#endif

#ifndef TEST_DENSITY
#  define TEST_DENSITY (0.5f)
#endif

