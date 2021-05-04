/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/pathFilters/BasePathFilter.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

namespace Belle2 {
  /// Filter for three hits using QualityEstimators plus a virtual IP.
  class TwoHitVirtualIPQIFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// initialize quality estimator
    void initialize() override;
    /// set BField value for estimator
    void beginRun() override;

  private:
    /// virtual IP SpacePoint
    SpacePoint m_virtualIPSpacePoint;
    /// cut on the POCA distance in xy obtained from the helixFitEstimator
    double m_param_helixFitPocaVirtIPDCut = 1.0;

    /// Identifier which estimation methsod to use. Valid identifiers are:
    /// mcInfo, tripletFit, helixFit
    std::string m_param_EstimationMethod = "helixFit";
    /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
    std::string m_param_MCRecoTracksStoreArrayName = "MCRecoTracks";
    /// Only required for MCInfo method
    bool m_param_MCStrictQualityEstimator = true;
    /// pointer to the selected QualityEstimator
    std::unique_ptr<QualityEstimatorBase> m_estimator;

    /// BeamSpot from DB
    DBObjPtr<BeamSpot> m_BeamSpotDB;
    /// Actual BeamSpot
    BeamSpot m_BeamSpot;
  };
}
