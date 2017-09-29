/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <tracking/trackFindingVXD/algorithms/SPTCSelectorBase.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>



namespace Belle2 {


  class SPTCSelectorMinLength : public SPTCSelectorBase {
  public:
    SPTCSelectorMinLength(unsigned short minPathLength):
      m_minPathLength(minPathLength),
      SPTCSelectorBase() {}

    void addSPTC(std::vector<SpacePointTrackCand>& target, SpacePointTrackCand cand) const override
    {
      // determine length
      if (cand.getNHits() < m_minPathLength) {
        // if long enough add to final set
        target.push_back(cand);
      }
    }
  private:
    unsigned short m_minPathLength;
  };
}
