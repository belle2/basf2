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
  class SPTCSelectorXBestPerFamily : public SPTCSelectorBase {
  public:
    /// Constructor
    SPTCSelectorXBestPerFamily(unsigned short xBest = 5):
      SPTCSelectorBase(), m_xBest(xBest)
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
        m_bestPaths.push_back(sptc);
        m_current_index++;
      } else if (m_bestPaths.at(m_familyIndex[family]).size() < m_xBest) {

      } else if (qi > m_bestPaths.at(m_familyIndex[family]).getQualityIndex()) {
        sptc.setQualityIndex(qi);
        m_bestPaths.at(m_familyIndex[family]) = sptc;
      }
    }

    void insert(std::vector<SpacePointTrackCand>& paths, SpacePointTrackCAnd sptc)
    {
      std::vector<SpacePointTrackCand>::iterator it = std::lower_bound(paths.begin(), paths.end(), sptc, std::greater<int>());
      cont.insert(it, value);   // insert before iterator it
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
    std::vector<std::vector<SpacePointTrackCand>> m_bestPaths;

    /** TODO: Add comment */
    std::vector<short> m_familyIndex;

    /** TODO: Add comment */
    unsigned short m_current_index = 0;

    /** TODO: Add comment */
    unsigned short m_XBest;
  };
}
