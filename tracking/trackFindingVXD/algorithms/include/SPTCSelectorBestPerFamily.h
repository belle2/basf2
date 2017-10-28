/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Felix Metzner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

#include <tracking/trackFindingVXD/algorithms/SPTCSelectorBase.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>


namespace Belle2 {
  class SPTCSelectorBestPerFamily : public SPTCSelectorBase {
  public:
    /// Constructor
    SPTCSelectorBestPerFamily():
      SPTCSelectorBase()
    {
      m_estimator = std::make_unique<QualityEstimatorTripletFit>();
    };

    /** Preparation of Best Candidate Selector by resetting the vectors. */
    void prepareSelector(unsigned short nFamilies)
    {
      m_bestPaths.clear();
      m_familyIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyIndex.resize(nFamilies, -1);

      m_current_index = 0;
    }

    /** Test new SPTC if it is better than the current best one and replace current one if so. */
    void testNewSPTC(SpacePointTrackCand sptc)
    {
      auto qi = m_estimator->estimateQuality(sptc.getSortedHits());
      short family = sptc.getFamily();

      if (m_familyIndex.at(family) == -1) {
        m_familyIndex.at(family) = m_current_index;
        sptc.setQualityIndex(qi);
        m_bestPaths.push_back(sptc);
        m_current_index++;
      } else if (qi > m_bestPaths.at(m_familyIndex[family]).getQualityIndex()) {
        sptc.setQualityIndex(qi);
        m_bestPaths.at(m_familyIndex[family]) = sptc;
      }
    }

    /** Return vector containing the best SPTCs; one per family. */
    std::vector<SpacePointTrackCand> returnSelection() const
    {
      return m_bestPaths;
    }


  private:
    /** Pinter to the Quality Estimator used to evaluate the SPTCs to find the best. */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** Vector of best SPTCs; one per family. */
    std::vector<SpacePointTrackCand> m_bestPaths;

    /** Map of family number to respective index for m_bestPaths */
    std::vector<short> m_familyIndex;

    /** Counter for current index which is increased each time a family is seen for the first time. */
    unsigned short m_current_index = 0;
  };
}
