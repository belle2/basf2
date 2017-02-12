/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/GetIterator.h>
#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Type predicate to check if Ts is iterable by means of std::begin().
     *  Result type is equivalent to std::true_type or does not compile,
     *  because it attempts to find the iterator type of the range or fails to do so.
     *  Therefore, the expression is currently limited to be used within enable_if type expressions.
     */
    template<class Ts>
    using IsIterable = std::integral_constant<bool, not std::is_void<GetIterator<Ts> >::value >;

  }
}
