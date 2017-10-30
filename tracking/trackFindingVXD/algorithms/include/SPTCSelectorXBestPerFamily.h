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

    /** Preparation of Best Candidate Selector by resetting the vectors. */
    void prepareSelector(unsigned short nFamilies)
    {
      m_bestPaths.clear();
      m_familyIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyIndex.resize(nFamilies, -1);

      m_current_index = 0;
    }

    /** Test new SPTC if it is better than the least best of the current x best SPTCs of its respective family.
     *  If so, the least best is thrown out, and the new is added to the sorted x best SPTC vector.
     *  If the maximal number of best SPTCs is not reached for the family, yet, the SPTC is just added at the right place.*/
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

    /** Return vector containing the best SPTCs; maximal m_xBest per family. */
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

    /** Function to insert SPTCs into a vector of SPTC while preserving an order by descending quality indices. */
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


    /** Pointer to the Quality Estimator used to evaluate the SPTCs to find the best. */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** Vector containing one vector of the best SPTCs per family. */
    std::vector<std::vector<SpacePointTrackCand>> m_bestPaths;

    /** Map of family number to respective index for m_bestPaths */
    std::vector<short> m_familyIndex;

    /** Counter for current index which is increased each time a family is seen for the first time. */
    unsigned short m_current_index = 0;

    /** Number of allowed best SPTCs per family. */
    unsigned short m_xBest;
  };
}
