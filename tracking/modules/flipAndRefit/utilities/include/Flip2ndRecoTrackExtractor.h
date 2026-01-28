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
    explicit FlipRecoTrackExtractor2nd(std::vector<TrackingUtilities::Named<float*>>& variableSet, const std::string& prefix = ""):
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
      // Set all values to NaN to make sure that no values from previous recoTrack are used accidentally
      setDefaultValues();

      const Track* track = recoTrack.getRelated<Track>("Tracks");
      const RecoTrack* flippedRecoTrack = recoTrack.getRelatedFrom<RecoTrack>("RecoTracks_flipped");
      if (not track or not flippedRecoTrack) {
        // Don't have track or flippedRecoTrack -> return
        return;
      }
      const Track* flippedTrack = flippedRecoTrack->getRelatedFrom<Track>("Tracks_flipped");
      if (not flippedTrack) {
        // Don't have flippedTrack -> return
        return;
      }
      const TrackFitResult* trackFitResult = track->getTrackFitResultWithClosestMass(Const::pion);
      const TrackFitResult* flippedTrackFitResult =
        flippedTrack->getTrackFitResultWithClosestMassByName(Const::pion, "TrackFitResults_flipped");
      if (not trackFitResult or not flippedTrackFitResult) {
        // Don't have trackFitResult or flippedTrackFitResult -> return
        return;
      }

      const auto& unflippedCovariance = trackFitResult->getCovariance6();
      const auto& unflippedMomentum = trackFitResult->getMomentum();
      const auto& unflippedPosition = trackFitResult->getPosition();

      const auto& flippedMomentum = flippedTrackFitResult->getMomentum();
      const auto& flippedPosition = flippedTrackFitResult->getPosition();

      m_variables.at(m_prefix + "flipped_pz_estimate") = static_cast<float>(flippedMomentum.Z());
      m_variables.at(m_prefix + "tan_lambda_estimate") = static_cast<float>(trackFitResult->getCotTheta());
      m_variables.at(m_prefix + "d0_variance") = static_cast<float>(trackFitResult->getCov()[0]);
      m_variables.at(m_prefix + "z_estimate") = static_cast<float>(unflippedPosition.Z());
      m_variables.at(m_prefix + "px_variance") = static_cast<float>(unflippedCovariance(3, 3));
      m_variables.at(m_prefix + "p_value") = static_cast<float>(trackFitResult->getPValue());
      m_variables.at(m_prefix + "pt_estimate") = static_cast<float>(unflippedMomentum.Rho());
      m_variables.at(m_prefix + "y_estimate") = static_cast<float>(unflippedPosition.Y());
      m_variables.at(m_prefix + "d0_estimate") = static_cast<float>(trackFitResult->getD0());
      m_variables.at(m_prefix + "x_estimate") = static_cast<float>(unflippedPosition.X());
      m_variables.at(m_prefix + "pz_variance") = static_cast<float>(unflippedCovariance(5, 5));
      m_variables.at(m_prefix + "omega_estimate") = static_cast<float>(trackFitResult->getOmega());
      m_variables.at(m_prefix + "quality_flip_indicator") = static_cast<float>(recoTrack.getFlipQualityIndicator());
      m_variables.at(m_prefix + "px_estimate") = static_cast<float>(unflippedMomentum.X());
      m_variables.at(m_prefix + "flipped_z_estimate") = static_cast<float>(flippedPosition.Z());
      m_variables.at(m_prefix + "py_estimate") = static_cast<float>(unflippedMomentum.Y());
      m_variables.at(m_prefix + "inGoingArmTime") = static_cast<float>(recoTrack.getIngoingArmTime());
      m_variables.at(m_prefix + "outGoingArmTime") = static_cast<float>(recoTrack.getOutgoingArmTime());
    }

  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  private:
    /// Set all variables to default error value
    void setDefaultValues()
    {
      m_variables.at(m_prefix + "flipped_pz_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "tan_lambda_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "d0_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "z_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "px_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "p_value") = Const::floatNaN;
      m_variables.at(m_prefix + "pt_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "y_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "d0_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "x_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "pz_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "omega_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "px_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "flipped_z_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "py_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "outGoingArmTime") = Const::floatNaN;
      m_variables.at(m_prefix + "quality_flip_indicator") = Const::floatNaN;
      m_variables.at(m_prefix + "inGoingArmTime") = Const::floatNaN;
    }
  };
}
