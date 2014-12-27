#pragma once

#include "framework/xmldb/config.h"

#ifndef MUST_USE_RESULT
#  ifdef __GNUC__
#    define MUST_USE_RESULT __attribute__ ((warn_unused_result))
#  else
#    define MUST_USE_RESULT
#  endif
#endif

//#ifndef HAVE_NULLPTR
//#  define nullptr NULL
//#endif

