/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorBaseModule.h>

namespace Belle2 {

  /** Quality estimation module for SpacePointTrackCandidates.
   * This module calculates a qualityIndex for each SpacePointTrackCandidate.
   * Following the Strategy pattern this module can support
   * all estimation strategies that implement the interface QualityEstimatorBase.
   *  */
  class QualityEstimatorVXDModule : public QualityEstimatorBaseModule {

  public:

    /** Constructor of the module. */
    QualityEstimatorVXDModule();

    virtual void beginRun() override;

    /** Applies the selected quality estimation method to SPTCs */
    virtual void singleSPTCevent(SpacePointTrackCand&) override;


  protected:

  };
}
