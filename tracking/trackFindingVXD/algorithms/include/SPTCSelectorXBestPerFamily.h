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
        B2DEBUG(100, "Setting index to " << m_current_index << " for family " << family << " and adding sptc with qi of " << qi);
        m_familyIndex.at(family) = m_current_index;
        m_bestPaths.push_back(std::vector<SpacePointTrackCand>(1));
        sptc.setQualityIndex(qi);
        m_bestPaths.at(m_familyIndex[family]).at(0) = sptc;
        m_current_index++;
      } else if (m_bestPaths.at(m_familyIndex[family]).size() < m_xBest) {
        B2DEBUG(100, "Adding new sptc with qi " << qi << " without check, as max lenght not reached, yet...");
        sptc.setQualityIndex(qi);
        insert(m_bestPaths.at(m_familyIndex[family]), sptc);
      } else if (qi > m_bestPaths.at(m_familyIndex[family]).back().getQualityIndex()) {
        B2DEBUG(100, "Adding new sptc with qi " << qi << " and throwing out last entry of vector...");
        sptc.setQualityIndex(qi);
        insert(m_bestPaths.at(m_familyIndex[family]), sptc);
        m_bestPaths.at(m_familyIndex[family]).pop_back();
      }
    }


    /** TODO: Add comment */
    void insert(std::vector<SpacePointTrackCand>& paths, SpacePointTrackCand sptc)
    {
      /// Determine position of new SPTC in sorted SPTC vector to keep it sorted
      std::vector<SpacePointTrackCand>::iterator it = std::lower_bound(paths.begin(), paths.end(), sptc,
      [](SpacePointTrackCand & lhs, SpacePointTrackCand rhs) {
        return lhs.getQualityIndex() > rhs.getQualityIndex();
      });
      /// Insert befor iterator it
      paths.insert(it, sptc);
    }


    /** TODO: Add comment */
    std::vector<SpacePointTrackCand> returnSelection() const
    {
      unsigned short finalSize = 0;
      for (auto set : m_bestPaths) {
        finalSize += set.size();
      }
      std::vector<SpacePointTrackCand> jointBestPaths;
      jointBestPaths.reserve(finalSize);
      for (auto set : m_bestPaths) {
        jointBestPaths.insert(jointBestPaths.end(), set.begin(), set.end());
      }

      return jointBestPaths;
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
    unsigned short m_xBest;
  };
}
