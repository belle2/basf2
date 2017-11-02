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
#include <numeric>

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

    /** Preparation of Best Candidate Selector by resetting the vectors. */
    void prepareSelector(unsigned short nFamilies)
    {
      m_bestPaths.clear();
      m_familyToIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyToIndex.resize(nFamilies, -1);

      m_currentIndex = 0;
    }

    /** Test new SPTC if it is better than the least best of the current x best SPTCs of its respective family.
     *  If so, the least best is thrown out, and the new is added to the sorted x best SPTC vector.
     *  If the maximal number of best SPTCs is not reached for the family, yet, the SPTC is just added at the right place.*/
    void testNewSPTC(SpacePointTrackCand& sptc)
    {
      auto qi = m_estimator->estimateQuality(sptc.getSortedHits());
      short family = sptc.getFamily();

      if (m_familyToIndex.at(family) == -1) {
        B2DEBUG(100, "Setting index to " << m_currentIndex << " for family " << family << " and adding sptc with qi of " << qi);
        m_familyToIndex.at(family) = m_currentIndex;
        sptc.setQualityIndex(qi);
        m_bestPaths.emplace_back(std::vector<SpacePointTrackCand> { sptc });
        m_currentIndex++;
      } else if (m_bestPaths.at(m_familyToIndex[family]).size() < m_xBest) {
        B2DEBUG(100, "Adding new sptc with qi " << qi << " without check, as max lenght not reached, yet...");
        sptc.setQualityIndex(qi);
        insertSortedByQI(m_bestPaths.at(m_familyToIndex[family]), sptc);
      } else if (qi > m_bestPaths.at(m_familyToIndex[family]).back().getQualityIndex()) {
        B2DEBUG(100, "Adding new sptc with qi " << qi << " and throwing out last entry of vector...");
        sptc.setQualityIndex(qi);
        insertSortedByQI(m_bestPaths.at(m_familyToIndex[family]), sptc);
        m_bestPaths.at(m_familyToIndex[family]).pop_back();
      }
    }

    /** Return vector containing the best SPTCs; maximal m_xBest per family. */
    std::vector<SpacePointTrackCand> returnSelection() const
    {
      std::vector<SpacePointTrackCand> jointBestPaths;
      jointBestPaths.reserve(std::accumulate(m_bestPaths.begin(), m_bestPaths.end(), 0,
      [](int a, std::vector<SpacePointTrackCand> b) { return a + b.size(); }));

      for (auto && set : m_bestPaths) {
        jointBestPaths.insert(jointBestPaths.end(), set.begin(), set.end());
      }

      return jointBestPaths;
    }


  private:

    /** Function to insert SPTCs into a vector of SPTC while preserving an order by descending quality indices. */
    void insertSortedByQI(std::vector<SpacePointTrackCand>& paths, SpacePointTrackCand sptc)
    {
      /// Determine position of new SPTC in sorted SPTC vector to keep it sorted
      std::vector<SpacePointTrackCand>::iterator it = std::lower_bound(paths.begin(), paths.end(), sptc,
      [](SpacePointTrackCand & lhs, SpacePointTrackCand rhs) {
        return lhs.getQualityIndex() > rhs.getQualityIndex();
      });
      /// Insert befor iterator it
      paths.insert(it, sptc);
    }


    /** Pointer to the Quality Estimator used to evaluate the SPTCs to find the best. */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** Vector containing one vector of the best SPTCs per family. */
    std::vector<std::vector<SpacePointTrackCand>> m_bestPaths;

    /** Map of family number to respective index for m_bestPaths */
    std::vector<short> m_familyToIndex;

    /** Counter for current index used for m_familyToIndex. The counter is increased each time a family is seen for
     *  the first time and thus a new entry for this family is added to m_familyToIndex. */
    unsigned short m_currentIndex = 0;

    /** Number of allowed best SPTCs per family. */
    unsigned short m_xBest;
  };
}
