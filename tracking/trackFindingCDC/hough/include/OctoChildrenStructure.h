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

#include <iterator>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Node>
    class OctoChildrenStructure {
      OctoChildrenStructure(const typename Node::Properies& box, std::size_t level)
      {
        for (int iX = 0 ; iX < 2 ; ++iX) {
          for (int iY = 0 ; iY < 2 ; ++iY) {

            for (int iZ = 0 ; iZ < 2 ; ++iZ) {
              m_nodes[iX][iY][iZ].setBounds
              (
                iX ? box.template getLowerHalfBounds<0>() : box.template getUpperHalfBounds<0>(),
                iY ? box.template getLowerHalfBounds<1>() : box.template getUpperHalfBounds<1>(),
                iZ ? box.template getLowerHalfBounds<2>() : box.template getUpperHalfBounds<2>()
              );
            }
          }
        }
      }

    private:
      Node m_nodes[2][2][2];

    public:
      auto begin() -> decltype(std::begin(m_nodes))
      {
        return std::begin(m_nodes);
      }

      auto end() -> decltype(std::end(m_nodes))
      {
        return std::begin(m_nodes);
      }
    };

  }
}
