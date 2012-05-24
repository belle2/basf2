// -*- C++ -*-
// CLASSDOC OFF
// $Id: CLHEP.h 10030 2007-03-09 07:51:44Z katayama $
// ---------------------------------------------------------------------------
// CLASSDOC ON
//
// This file is a part of the CLHEP - a Class Library for High Energy Physics.
//
// This file contains definitions of some usefull utilities and macros.
//
#ifndef BELLE_CLHEP_CLHEP_H
#define BELLE_CLHEP_CLHEP_H

//iw
#if defined(BELLE_CLHEP_TARGET_H)
#include BELLE_CLHEP_TARGET_H
#else
//iw #include "belleCLHEP/config/CLHEP-default.h"
#include "tracking/modules/trasan/CLHEP-default.h"
#endif

namespace Belle {

//#include <algorithm>

// CLASSDOC OFF
// **** You should probably not touch anything below this line: ****

  typedef double HepDouble;
  typedef int    HepInt;
  typedef float  HepFloat;

  typedef bool HepBoolean;

#ifdef HEP_SHORT_NAMES
  typedef HepBoolean Boolean;
#endif

#ifndef M_PI_4
#define M_PI_4  0.78539816339744830962
#endif

#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923
#endif

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#ifndef M_2PI
#define M_2PI   6.28318530717958647692
#endif

#ifndef M_1_PI
#define M_1_PI  0.31830988618379067154
#endif



#ifndef CLHEP_SQR_DEFINED
#define CLHEP_SQR_DEFINED
  template <class T>
  inline T sqr(const T& x)
  {
    return x * x;
  }
#endif

#ifndef CLHEP_ABS_DEFINED
#define CLHEP_ABS_DEFINED
#ifndef HEP_ABS_NEEDS_PARAMETER_WITHOUT_CONST
  template <class T>
  inline T abs(const T& a)
  {
    return a < 0 ? -a : a;
  }
#else
  template <class T>
  inline T abs(T a)
  {
    return a < 0 ? -a : a;
  }
#endif /* HEP_ABS_NEEDS_PARAMETER_WITHOUT_CONST */
#endif

#ifdef HEP_DEBUG_INLINE
#define HEP_NO_INLINE_IN_DECLARATION
#endif

#ifdef HEP_NO_INLINE_IN_DECLARATION
#define HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#endif

// Default to generate random matrix
//
#ifndef HEP_USE_RANDOM
#define HEP_USE_RANDOM
#endif

// GNU g++ compiler can optimize when returning an object.
// However g++ on HP cannot deal with this.
//
#undef HEP_GNU_OPTIMIZED_RETURN

// CLASSDOC ON

} // namespace Belle

#endif /* CLHEP_CLHEP_H */
