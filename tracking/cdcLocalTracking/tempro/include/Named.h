/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef NAMED_H
#define NAMED_H

#include <stddef.h>

#include "pp_pow.h"
#include "StaticString.h"

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

#ifdef NAMED

#   error "Macro 'NAMED' defined before"

#else
/// Exponent (taken for a power of two) of the maximal string length given to the NAMED macro
#   define NAMED_MAXLENGTH_EXPONENT 6

/// The maximal string length that can be used in the NAMED macro - currently 64
#   define NAMED_MAXLENGTH BELLE2_PP_POW(2, NAMED_MAXLENGTH_EXPONENT) //Including the terminating '\0' character

/// Helper macro for the repeatition of the string unpacking in the NAMED macro
#   define __NAMED_AT(Z,i,name) ::Belle2::CDCLocalTracking::Named::at_limited(name,i)

//#   define NAMED(name) StaticStripBack<at(name,0), at(name,1), at(name,2), at(name,3), at(name,4), at(name,5), at(name,6)>::type

/// Macro turning a string literal to a StaticString class representing that string as a type.
#   define NAMED(name) StaticStripBack<BOOST_PP_ENUM(NAMED_MAXLENGTH, __NAMED_AT, name)>::type

#endif

namespace Belle2 {
  namespace CDCLocalTracking {
    namespace Named {

      /// Method to retrieve a character from a char string safely. Returns '\0' for indices out of range.
      template <size_t N>
      constexpr char at(char const(&s)[N], size_t i)
      {
        return i >= N ? '\0' : s[i];
      }

      /** Method to retrieve a character from a char string safely. Returns '\0' for indices out of range.
       *  This puts a limit on the strings that can be passed to statically fail for strings longer than
       *  NAMED_MAXLENGTH characters, hence limiting the string length such that it can be safely used
       *  with the NAMED macro.*/
      template <size_t N>
      constexpr char at_limited(char const(&s)[N], size_t i)
      {
        static_assert(N <  NAMED_MAXLENGTH, "NAMED() cannot constrution StaticStrings longer than " BOOST_PP_STRINGIZE(NAMED_MAXLENGTH) " characters");
        return at(s, i);
      }

    } //end namespace Named
  } // end namespace CDCLocalTracking
} // namespace Belle2


#endif // NAMED_H
