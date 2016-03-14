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

#include <tracking/trackFindingCDC/utilities/GenIndices.h>
#include <tracking/trackFindingCDC/utilities/Product.h>

#include <tuple>
#include <array>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Factory object that constructs sub boxes from a given box with optional overlaps.
     *  The number of divisions in each dimension are given as template parameters such
     *  that the total number is know at compile time.
     */
    template<class ABox, std::size_t... divisions>
    class LinearDivision {

    public:
      /// Number of sub boxes produced by this factory facility.
      static const std::size_t s_nSubBoxes = Product<divisions...>::value;

    private:
      /// Array of the number of divisions for each dimension.
      static constexpr std::size_t s_divisions[sizeof...(divisions)] = {divisions...};

    public:
      /// Initialise the sub box factory with specific overlaps.
      LinearDivision(const typename ABox::Delta& overlaps = typename ABox::Delta()) :
        m_overlaps(overlaps)
      {}

    public:
      /// Factory method to construct the subboxes with overlap from the given box.
      std::array<ABox, s_nSubBoxes> operator()(const ABox& box)
      {
        return makeSubBoxes(box, GenIndices<s_nSubBoxes>());
      }

      /// Make all subboxs with overlap of the given box.
      template<std::size_t... Is>
      inline
      std::array<ABox, s_nSubBoxes>
      makeSubBoxes(const ABox& box, IndexSequence<Is...> /*globalSubBoxIndex*/)
      {
        return {{ makeSubBox(box, Is, GenIndices<sizeof...(divisions)>())... }};
      }


      /// Make the subbox with overlaps of the given box at global index.
      template<std::size_t... Is>
      inline
      ABox makeSubBox(const ABox& box,
                      std::size_t globalISubBox,
                      IndexSequence<Is...> /*coordinatesIndex*/)
      {
        std::array<std::size_t, sizeof...(divisions)> indices;
        for (size_t iIndex = 0 ; iIndex <  sizeof...(divisions); ++iIndex) {
          indices[iIndex] = globalISubBox % s_divisions[iIndex];
          globalISubBox /= s_divisions[iIndex];
        }
        assert(globalISubBox == 0);
        return ABox(box.template getDivisionBoundsWithOverlap<Is>(std::get<Is>(m_overlaps),
                                                                  s_divisions[Is],
                                                                  indices[Is]) ...);
      }

    private:
      /// Custom overlaps of the bounds at each division for each dimension.
      typename ABox::Delta m_overlaps;

    };

    // Extra mention of the constexpr such that it aquires external linkage.
    template<class ABox, std::size_t... divisions>
    constexpr std::size_t LinearDivision<ABox, divisions...>::s_divisions[sizeof...(divisions)];
  }
}
