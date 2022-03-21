/*
 * common.h - common includes and parameters
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_COMMON_H
#define QUL_COMMON_H

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif /* HAVE_CONFIG_H */

#include <complex>
#include <limits>
#include <inttypes.h>
// #include <stdint.h>

#include <cassert>
#ifndef NDEBUG
#include <cstdio> // DEBUG
#endif

typedef std::complex<double> ncomplex;
// typedef long long int int64_t;

// Forward declarations
class ICache;

class MCache;
class MCache5;

class Minor5;

class Kinem5;
class Kinem4;
class Kinem3;
class Kinem2;

#define CONST __attribute__ ((const))
#define PURE __attribute__ ((pure))

#ifdef USE_GOLEM_MODE
#   define USE_ZERO_CHORD       "1" /* calculate formfactors for zero-chord */
#   define USE_GOLEM_MODE_6     "1" /* support 6-pinched kinematics         */
#   define USE_GOLEM_ZERO_CHECK "1" /* enable checks for i,j,k==s,t,u */
#endif

// Approx 1% slowdown
#define USE_SMART_INSERT        "1" /* before adding new, delete same old entries in MCache2,3 */

#define USE_DIRTY_RESET         "1" /* when told to clear caches, just set len=0 */

#endif /* QUL_COMMON_H */

