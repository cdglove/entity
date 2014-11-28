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

