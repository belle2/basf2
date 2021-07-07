/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/** \def BITMASK a simplified version of boost/detail/bitmask.hpp */
#define ADD_BITMASK_OPERATORS(Bitmask) \
  \
  inline Bitmask operator| (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<int>(x)     \
                                 | static_cast<int>(y)); }                          \
  \
  inline Bitmask operator& (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<int>(x)     \
                                 & static_cast<int>(y)); }                          \
  \
  inline Bitmask operator^ (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<int>(x)     \
                                 ^ static_cast<int>(y)); }                          \
  \
  inline Bitmask operator~ (Bitmask x )                                   \
  { return static_cast<Bitmask>(~static_cast<int>(x)); } \
  \
  inline Bitmask & operator&=(Bitmask & x , Bitmask y)                    \
  { x = x & y ; return x ; }                                              \
  \
  inline Bitmask & operator|=(Bitmask & x , Bitmask y)                    \
  { x = x | y ; return x ; }                                              \
  \
  inline Bitmask & operator^=(Bitmask & x , Bitmask y)                    \
  { x = x ^ y ; return x ; }
