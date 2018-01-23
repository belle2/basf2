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

#include <tracking/trackFindingCDC/utilities/Product.h>

#include <framework/utilities/Utils.h>

#include <tuple>
#include <utility>
#include <vector>
#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Factory object that constructs sub boxes from a given box with optional overlaps.
     *
     *  In contrast to the bare LinearDivsion this division strategy may introduce a
     *  finer division on the first level leading to finer 'sectors' to be searched
     *  from the start. It effectively skips a number of levels a yields the binning further
     *  down the usual division level as level 1.
     *
     *  In this way some repeated computations in the first levels are abolished and
     *  a better overview which part of the hough space is more densly populared is aquired.
     *  The latter means that a better search space prioritisation can be expected compared
     *  to deciding the priority from coarse first level nodes.
     *
     *  Note since the tree using this division strategy is generally unaware of the fast forward
     *  the division granularity and the level number have a different relation.
     *  Essentially the sectorLevelSkip parameter setted here has to be added to the level number
     *  to know at which granularity one currently is in the tree.
     *  Alternatively the search depth can be reduced by sectorLevelSkip, which is what we do in
     *  the application.
     */
    template<class ABox, std::size_t... divisions>
    class SectoredLinearDivision {

    public:
      /// Number of sub boxes produced by this factory facility.
      static const std::size_t s_nSubBoxes = Product<divisions...>::value;

    private:
      /// Array of the number of divisions for each dimension.
      static constexpr std::size_t s_divisions[sizeof...(divisions)] = {divisions...};

    public:
      /// Initialise the sub box factory with specific overlaps.
      explicit SectoredLinearDivision(const typename ABox::Delta& overlaps = typename ABox::Delta(),
                                      int sectorLevelSkip = 0)
        : m_overlaps(overlaps)
        , m_sectorLevelSkip(sectorLevelSkip)
      {
      }

    public:
      /// Factory method to construct the subboxes with overlap from the given box.
      template <class ANode>
      std::vector<ABox> operator()(const ANode& node)
      {
        const ABox& box = node;
        if (branch_unlikely(node.getLevel() == 0)) {
          std::vector<ABox> result = makeSubBoxes(box, std::make_index_sequence<s_nSubBoxes>());
          // Apply further divisions sectorLevelSkip times and return all boxes.
          for (int iSkipped = 0; iSkipped < m_sectorLevelSkip; ++iSkipped) {
            std::vector<ABox> sectoredBoxes;
            sectoredBoxes.reserve(result.size() * s_nSubBoxes);
            for (const ABox& boxToDivide : result) {
              for (const ABox& dividedBox : makeSubBoxes(boxToDivide, std::make_index_sequence<s_nSubBoxes>())) {
                sectoredBoxes.push_back(dividedBox);
              }
            }
            result = std::move(sectoredBoxes);
          }

          return result;
        }
        return makeSubBoxes(box, std::make_index_sequence<s_nSubBoxes>());
      }

      /// Make all subboxs with overlap of the given box.
      template<std::size_t... Is>
      std::vector<ABox>
      makeSubBoxes(const ABox& box, std::index_sequence<Is...> /*globalSubBoxIndex*/)
      {
        return {{ makeSubBox(box, Is, std::make_index_sequence<sizeof...(divisions)>())... }};
      }

      /// Make the subbox with overlaps of the given box at global index.
      template <std::size_t... Is>
      ABox makeSubBox(const ABox& box, std::size_t globalISubBox, std::index_sequence<Is...> /*coordinatesIndex*/)
      {
        std::array<std::size_t, sizeof...(divisions)> indices;
        for (size_t c_Index = 0 ; c_Index <  sizeof...(divisions); ++c_Index) {
          indices[c_Index] = globalISubBox % s_divisions[c_Index];
          globalISubBox /= s_divisions[c_Index];
        }
        assert(globalISubBox == 0);
        return ABox(box.template getDivisionBoundsWithOverlap<Is>(std::get<Is>(m_overlaps),
                                                                  s_divisions[Is],
                                                                  indices[Is]) ...);
      }

    private:
      /// Custom overlaps of the bounds at each division for each dimension.
      typename ABox::Delta m_overlaps;

      /// Number of levels to be skipped to form the finer binning at level 1
      int m_sectorLevelSkip;
    };

    // Extra mention of the constexpr such that it aquires external linkage.
    template<class ABox, std::size_t... divisions>
    constexpr std::size_t SectoredLinearDivision<ABox, divisions...>::s_divisions[sizeof...(divisions)];
  }
}
