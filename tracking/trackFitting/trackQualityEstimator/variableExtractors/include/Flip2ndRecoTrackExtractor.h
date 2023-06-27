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
    void extractVariables(const RecoTrack& recoTrack)
    {

      m_variables.at(m_prefix + "inGoingArmTime") = recoTrack.getIngoingArmTime();
      m_variables.at(m_prefix + "outGoingArmTime") = recoTrack.getOutgoingArmTime();

      auto genfitTrack = recoTrack.getRelated<Track>("Tracks");

      if (genfitTrack) {
        auto trackFitResult = genfitTrack->getTrackFitResultWithClosestMass(Const::pion);

        RecoTrack* RecoTrack_flipped = recoTrack.getRelatedFrom<RecoTrack>("RecoTracks_flipped");
        if (RecoTrack_flipped) {
          Track* b2track_flipped = RecoTrack_flipped->getRelatedFrom<Track>("Tracks_flipped");
          if (b2track_flipped) {
            auto fitTrack_flipped = b2track_flipped->getTrackFitResultWithClosestMassByName(Const::pion, "TrackFitResults_flipped");
            if (trackFitResult && fitTrack_flipped) {
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
              float pt_estimate = mom.Rho();
              float pt_variance = (pow(mom.X(), 2) * cov6(3, 3) + pow(mom.Y(), 2) * cov6(4, 4) - 2 * mom.X() * mom.Y() * cov6(3,
                                   4)) / mom.Perp2();
              float pt_resolution = pt_variance / pt_estimate;

              float flipped_pt_variance = (pow(mom_flipped.X(), 2) * cov6_flipped(3, 3) + pow(mom_flipped.Y(), 2) * cov6_flipped(4,
                                           4) - 2 * mom_flipped.X() * mom_flipped.Y() * cov6_flipped(3, 4)) / mom_flipped.Perp2();

              m_variables.at(m_prefix + "px_estimate") = mom.X();
              m_variables.at(m_prefix + "flipped_z_estimate") = pos_flipped.Z();
              m_variables.at(m_prefix + "py_estimate") = mom.Y();

            }
          }
        }
      }

    }
  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  };
}
