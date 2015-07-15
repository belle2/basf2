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

#include <tracking/trackFindingCDC/hough/GenIndices.h>
#include <tracking/trackFindingCDC/hough/Product.h>

#include <array>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Box_, size_t... divisions>
    class LinearDivision {

    public:
      static const std::size_t s_nSubBoxes = Product<divisions...>::value;

    private:
      static constexpr const std::array<std::size_t, sizeof...(divisions)> s_divisions =
      {{divisions...}};

    public:
      std::array<Box_, s_nSubBoxes> operator()(const Box_& box)
      {
        return makeSubBoxes(box, GenIndices<s_nSubBoxes>());
      }

      template<std::size_t... Is>
      inline static std::array<Box_, s_nSubBoxes>
      makeSubBoxes(const Box_& box, IndexSequence<Is...> /*globalSubBoxIndex*/)
      {
        return {{ makeSubBox(box, Is, GenIndices<sizeof...(divisions)>())... }};
      }

      template<std::size_t... Is>
      inline static Box_ makeSubBox(const Box_& box,
                                    std::size_t globalISubBox,
                                    IndexSequence<Is...> /*coordinatesIndex*/)
      {
        std::array<std::size_t, sizeof...(divisions)> indices;
        for (size_t iIndex = 0 ; iIndex <  sizeof...(divisions); ++iIndex) {
          indices[iIndex] = globalISubBox % s_divisions[iIndex];
          globalISubBox /= s_divisions[iIndex];
        }
        assert(globalISubBox == 0);
        return Box_(box.template getDivisionBounds<Is>(s_divisions[Is], indices[Is]) ...);
      }
    };

    template<class Box_, std::size_t... divisions>
    const std::array<std::size_t, sizeof...(divisions)> LinearDivision<Box_, divisions...>::s_divisions;
  }
}
