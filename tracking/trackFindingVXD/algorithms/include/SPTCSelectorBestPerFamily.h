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
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>


namespace Belle2 {
  class SPTCSelectorBestPerFamily : public SPTCSelectorBase {
  public:
    SPTCSelectorBestPerFamily(std::unique_ptr<QualityEstimatorBase> estimator):
      SPTCSelectorBase(), m_estimator(estimator) {};

    void addSPTC(std::vector<SpacePointTrackCand>& target, SpacePointTrackCand sptc) const override
    {
      auto qi = m_estimator->estimateQuality(sptc.getSortedHits());

      SpacePointTrackCand& entry = target.at(sptc.getFamily());
      if (entry.getQualityIndex() < qi) {
        entry = sptc;
      }
    }
  private:
    std::unique_ptr<QualityEstimatorBase> m_estimator;
  };
}
