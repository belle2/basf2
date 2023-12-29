/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <genfit/FitStatus.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class FlipRecoTrackExtractor2nd : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit FlipRecoTrackExtractor2nd(std::vector<Named<float*>>& variableSet, const std::string& prefix = ""):
      VariableExtractor(), m_prefix(prefix)
    {
      addVariable(prefix + "flipped_pz_estimate", variableSet);
      addVariable(prefix + "tan_lambda_estimate", variableSet);
      addVariable(prefix + "d0_variance", variableSet);
      addVariable(prefix + "z_estimate", variableSet);
      addVariable(prefix + "px_variance", variableSet);
      addVariable(prefix + "p_value", variableSet);
      addVariable(prefix + "pt_estimate", variableSet);
      addVariable(prefix + "y_estimate", variableSet);
      addVariable(prefix + "d0_estimate", variableSet);
      addVariable(prefix + "x_estimate", variableSet);
      addVariable(prefix + "pz_variance", variableSet);
      addVariable(prefix + "omega_estimate", variableSet);
      addVariable(prefix + "px_estimate", variableSet);
      addVariable(prefix + "flipped_z_estimate", variableSet);
      addVariable(prefix + "py_estimate", variableSet);
      addVariable(prefix + "outGoingArmTime", variableSet);
      addVariable(prefix + "quality_flip_indicator", variableSet);
      addVariable(prefix + "inGoingArmTime", variableSet);
    }
    /// extract the actual variables and write into a variable set
    void extractVariables(RecoTrack& recoTrack)
    {
      auto genfitTrack = recoTrack.getRelated<Track>("Tracks");
      RecoTrack* RecoTrack_flipped = recoTrack.getRelatedFrom<RecoTrack>("RecoTracks_flipped");
      if (not genfitTrack or not RecoTrack_flipped) {
        // Don't have genfitTrack or RecoTrack_flipped -> set default values and return
        setDefaultValues();
        return;
      }
      Track* b2track_flipped = RecoTrack_flipped->getRelatedFrom<Track>("Tracks_flipped");
      if (not b2track_flipped) {
        // Don't have b2track_flipped -> set default values and return
        setDefaultValues();
        return;
      }
      auto trackFitResult = genfitTrack->getTrackFitResultWithClosestMass(Const::pion);
      auto fitTrack_flipped = b2track_flipped->getTrackFitResultWithClosestMassByName(Const::pion, "TrackFitResults_flipped");
      if (not trackFitResult or not fitTrack_flipped) {
        // Don't have trackFitResult or fitTrack_flipped -> set default values and return
        setDefaultValues();
        return;
      }

      auto cov6 = trackFitResult->getCovariance6();
      auto mom = trackFitResult->getMomentum();
      auto pos = trackFitResult->getPosition();

      auto cov6_flipped = fitTrack_flipped->getCovariance6();
      auto mom_flipped = fitTrack_flipped->getMomentum();
      auto pos_flipped = fitTrack_flipped->getPosition();

      m_variables.at(m_prefix + "flipped_pz_estimate") = mom_flipped.Z();
      m_variables.at(m_prefix + "tan_lambda_estimate") = trackFitResult->getCotTheta();
      m_variables.at(m_prefix + "d0_variance") = trackFitResult->getCov()[0];
      m_variables.at(m_prefix + "z_estimate") = pos.Z();
      m_variables.at(m_prefix + "px_variance") = cov6(3, 3);
      m_variables.at(m_prefix + "p_value") = trackFitResult->getPValue();
      m_variables.at(m_prefix + "pt_estimate") = mom.Rho();
      m_variables.at(m_prefix + "y_estimate") = pos.Y();
      m_variables.at(m_prefix + "d0_estimate") = trackFitResult->getD0();
      m_variables.at(m_prefix + "x_estimate") = pos.X();
      m_variables.at(m_prefix + "pz_variance") = cov6(5, 5);
      m_variables.at(m_prefix + "omega_estimate") = trackFitResult->getOmega();
      m_variables.at(m_prefix + "quality_flip_indicator") = recoTrack.getFlipQualityIndicator();
      m_variables.at(m_prefix + "px_estimate") = mom.X();
      m_variables.at(m_prefix + "flipped_z_estimate") = pos_flipped.Z();
      m_variables.at(m_prefix + "py_estimate") = mom.Y();
      m_variables.at(m_prefix + "inGoingArmTime") = recoTrack.getIngoingArmTime();
      m_variables.at(m_prefix + "outGoingArmTime") = recoTrack.getOutgoingArmTime();
    }

  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  private:
    /// Set all variables to default error value
    void setDefaultValues()
    {
      const float errorvalue = -99999.9;
      m_variables.at(m_prefix + "flipped_pz_estimate") = errorvalue;
      m_variables.at(m_prefix + "tan_lambda_estimate") = errorvalue;
      m_variables.at(m_prefix + "d0_variance") = errorvalue;
      m_variables.at(m_prefix + "z_estimate") = errorvalue;
      m_variables.at(m_prefix + "px_variance") = errorvalue;
      m_variables.at(m_prefix + "p_value") = errorvalue;
      m_variables.at(m_prefix + "pt_estimate") = errorvalue;
      m_variables.at(m_prefix + "y_estimate") = errorvalue;
      m_variables.at(m_prefix + "d0_estimate") = errorvalue;
      m_variables.at(m_prefix + "x_estimate") = errorvalue;
      m_variables.at(m_prefix + "pz_variance") = errorvalue;
      m_variables.at(m_prefix + "omega_estimate") = errorvalue;
      m_variables.at(m_prefix + "px_estimate") = errorvalue;
      m_variables.at(m_prefix + "flipped_z_estimate") = errorvalue;
      m_variables.at(m_prefix + "py_estimate") = errorvalue;
      m_variables.at(m_prefix + "outGoingArmTime") = errorvalue;
      m_variables.at(m_prefix + "quality_flip_indicator") = errorvalue;
      m_variables.at(m_prefix + "inGoingArmTime") = errorvalue;
    }
  };
}
