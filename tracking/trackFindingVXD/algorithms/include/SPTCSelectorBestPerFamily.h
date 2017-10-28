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

    /** TODO: Add comment */
    void prepareSelector(unsigned short nFamilies)
    {
      m_bestPaths.clear();
      m_familyIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyIndex.resize(nFamilies, -1);

      m_current_index = 0;
    }

    /** TODO: Add comment */
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

    /** TODO: Add comment */
    std::vector<SpacePointTrackCand> returnSelection() const
    {
      return m_bestPaths;
    }


  private:
    /** TODO: Add comment */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** TODO: Add comment */
    std::vector<SpacePointTrackCand> m_bestPaths;

    /** TODO: Add comment */
    std::vector<short> m_familyIndex;

    /** TODO: Add comment */
    unsigned short m_current_index = 0;
  };
}
