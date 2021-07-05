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

#include <tuple>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Apply the template function F to each element of the index sequence.
    template< template<size_t> class AGenerator, class AIndexSequence>
    struct TupleGenerateImpl {};

    /// Specialisation for concrete indices
    template< template<size_t> class AGenerator, size_t ... Is>
    struct TupleGenerateImpl<AGenerator, std::index_sequence<Is...> > {
      /// Result sequence of the function application.
      using type = std::tuple<AGenerator<Is>...>;
    };

    /// Generate a tuple from the given indices by applying the templated generator to them.
    template< template<size_t> class AFunction, class AIndexSequence>
    using TupleGenerate = typename TupleGenerateImpl<AFunction, AIndexSequence>::type;

    /// Generate a tuple from first N indices by applying the templated generator to them.
    template< template<size_t> class AFunction, size_t N>
    using TupleGenerateN = TupleGenerate<AFunction, std::make_index_sequence<N> >;
  }
}
