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

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>


namespace Belle2 {
  /**
   * Algorithm to collect the x best TrackCandidates per family based on a VXD Quality estimator method output.
   */
  class SPTCSelectorXBestPerFamily {

  public:

    /** Constructor
     * for the selection of the x best candidates for each family based on the quality index.
     * @param xBest Maximal number of best candidates to be stored per family.
     * @param estimationMethod Quality estimator to be used.
     */
    SPTCSelectorXBestPerFamily(unsigned short xBest = 5, std::string estimationMethod = std::string("tripletFit")):
      m_xBest(xBest)
    {
      if (estimationMethod == "tripletFit") {
        m_estimator = std::make_unique<QualityEstimatorTripletFit>();
      } else if (estimationMethod == "circleFit") {
        m_estimator = std::make_unique<QualityEstimatorCircleFit>();
      } else if (estimationMethod == "helixFit") {
        m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
      } else if (estimationMethod == "random") {
        m_estimator = std::make_unique<QualityEstimatorRandom>();
      }
      B2ASSERT("QualityEstimator could not be initialized with method: " << estimationMethod, m_estimator);
    };

    /// Destructor
    ~SPTCSelectorXBestPerFamily() = default;

    /** Preparation of Best Candidate Selector by resetting the vectors.
     * To be executed at the beginning of a new event after the families are defined to reset the respective vectors
     * and set them up under consideration of the number of families in the event.
     * @param nFamilies Number of families in the event.
     */
    void prepareSelector(unsigned short nFamilies)
    {
      m_bestPaths.clear();
      m_familyToIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyToIndex.resize(nFamilies, -1);

      m_currentIndex = 0;
    }

    /** Test new SPTC if it is better than the least best of the current x best SPTCs of its respective family.
     *  If so, the least best is thrown out, and the new is added to the sorted x best SPTC multiset of the family.
     *  If the maximal number of best SPTCs is not reached for the family, yet, the SPTC is just added at the right place.
     *  @param sptc SpacePointTrackCandidate to be evaluated for adding.
     */
    void testNewSPTC(SpacePointTrackCand& sptc)
    {
      sptc.setQualityIndex(m_estimator->estimateQuality(sptc.getSortedHits()));
      short family = sptc.getFamily();

      if (m_familyToIndex.at(family) == -1) {
        m_bestPaths.emplace_back(std::multiset<SpacePointTrackCand> { sptc });
        m_familyToIndex.at(family) = m_currentIndex;
        m_currentIndex++;
        return;
      }

      auto& currentSet = m_bestPaths.at(m_familyToIndex[family]);

      if (currentSet.size() == m_xBest) {
        std::multiset<SpacePointTrackCand>::iterator iter = currentSet.begin();
        if (sptc.getQualityIndex() < iter->getQualityIndex()) {
          return;
        }
        currentSet.erase(iter);
      }
      currentSet.emplace_hint(currentSet.cbegin(), sptc);
    }

    /** Return vector containing the best SPTCs; maximal m_xBest per family.
     * @returns One vector containing the best SPTC candidates for the whole event.
     */
    std::vector<SpacePointTrackCand> returnSelection() const
    {
      std::vector<SpacePointTrackCand> jointBestPaths;
      jointBestPaths.reserve(std::accumulate(m_bestPaths.begin(), m_bestPaths.end(), 0,
      [](int a, auto b) { return a + b.size(); }));

      for (auto && set : m_bestPaths) {
        jointBestPaths.insert(jointBestPaths.end(), set.begin(), set.end());
      }

      return jointBestPaths;
    }

    /** Setting magnetic field for the quality estimator.
     * @param bFieldZ Magnetic Field value to be used for the Quality Estimation.
     */
    void setMagneticFieldForQE(double bFieldZ)
    {
      m_estimator->setMagneticFieldStrength(bFieldZ);
    }

  private:

    /** Pointer to the Quality Estimator used to evaluate the SPTCs to find the best. */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /** Vector containing one vector of the best SPTCs per family. */
    std::vector<std::multiset<SpacePointTrackCand> > m_bestPaths;

    /** Map of family number to respective index for m_bestPaths */
    std::vector<short> m_familyToIndex;

    /** Counter for current index used for m_familyToIndex. The counter is increased each time a family is seen for
     *  the first time and thus a new entry for this family is added to m_familyToIndex. */
    unsigned short m_currentIndex = 0;

    /** Number of allowed best SPTCs per family. */
    unsigned short m_xBest;
  };
}
