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
#include <cstddef>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Compile time helper object representing a sequence of indices.
     *  Helps in looping over std::tuple objects.
     */
    template<size_t... indices>
    struct IndexSequence {};

    /** Template to generate a continuous sequence of indices
     *  Implementation accumulates already generated indices on the right hand side
     *  while the first number determines the number of indices that still need to be
     *  expanded.
     *  Specialisation for the terminating case is given below.
     */
    template<size_t remainingNIndices, size_t... indices>
    struct GenIndicesImpl : GenIndicesImpl < remainingNIndices - 1, remainingNIndices - 1, indices... > { };

    /** Specialisation terminating the recursion
     *  where the number of indices that still need to be expanded drops to zero
     */
    template<size_t ...indices>
    struct GenIndicesImpl<0, indices...> {
      /// Result sequence.
      using Type = IndexSequence<indices...>;
    };

    /// Short for the GenIndicesImpl that executes the expansion without the typename and ::Type
    template<size_t nIndices>
    using GenIndices = typename GenIndicesImpl<nIndices>::Type;


    /// Apply the template function F to each element of the index sequence.
    template< template<size_t> class AFunction, class AIndexSequence>
    struct MapIndicesImpl {};

    /// Specialisation for concrete indices
    template< template<size_t> class AFunction, size_t ... Is>
    struct MapIndicesImpl<AFunction, IndexSequence<Is...> > {
      /// Result sequence of the function application.
      typedef std::tuple<AFunction<Is>...> Type;
    };

    /// Generate the first N indices and apply the template function to them.
    template< template<size_t> class AFunction, size_t N>
    using MapGenIndices = typename MapIndicesImpl<AFunction, GenIndices<N> >::Type;

  }
}
