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
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class FlipRecoTrackExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit FlipRecoTrackExtractor(std::vector<Named<float*>>& variableSet, const std::string& prefix = ""):
      VariableExtractor(), m_prefix(prefix)
    {
      /// Adding variables for 1st mva
      // addVariable(prefix + "d0_variance", variableSet);
      addVariable(prefix + "seed_pz_estimate", variableSet);
      // addVariable(prefix + "n_hits", variableSet);
      // addVariable(prefix + "z0_estimate", variableSet);
      addVariable(prefix + "seed_pz_variance", variableSet);
      // addVariable(prefix + "phi0_variance", variableSet);
      addVariable(prefix + "seed_z_estimate", variableSet);
      // addVariable(prefix + "tan_lambda_estimate", variableSet);
      // addVariable(prefix + "omega_variance", variableSet);
      addVariable(prefix + "seed_tan_lambda_estimate", variableSet);
      // addVariable(prefix + "d0_estimate", variableSet);
      addVariable(prefix + "seed_pt_estimate", variableSet);
      // addVariable(prefix + "cdc_qualityindicator", variableSet);
      // addVariable(prefix + "omega_estimate", variableSet);
      // addVariable(prefix + "z0_variance", variableSet);
      addVariable(prefix + "seed_x_estimate", variableSet);
      addVariable(prefix + "seed_y_estimate", variableSet);
      // addVariable(prefix + "seed_pt_resolution", variableSet);
      addVariable(prefix + "seed_py_variance", variableSet);
      addVariable(prefix + "seed_d0_estimate", variableSet);
      addVariable(prefix + "seed_omega_variance", variableSet);
      // addVariable(prefix + "tan_lambda_variance", variableSet);
      addVariable(prefix + "svd_layer6_clsTime", variableSet);
      addVariable(prefix + "seed_tan_lambda_variance", variableSet);
      addVariable(prefix + "seed_z_variance", variableSet);
      addVariable(prefix + "n_svd_hits", variableSet);
      // addVariable(prefix + "phi0_estimate", variableSet);
      addVariable(prefix + "n_cdc_hits", variableSet);
      addVariable(prefix + "svd_layer3_positionSigma", variableSet);
      addVariable(prefix + "first_cdc_layer", variableSet);
      addVariable(prefix + "last_cdc_layer", variableSet);
      // addVariable(prefix + "ndf_hits", variableSet);
      addVariable(prefix + "InOutArmTimeDifference", variableSet);
      addVariable(prefix + "InOutArmTimeDifferenceError", variableSet);
      addVariable(prefix + "inGoingArmTime", variableSet);
      addVariable(prefix + "inGoingArmTimeError", variableSet);
      addVariable(prefix + "outGoingArmTime", variableSet);
      addVariable(prefix + "outGoingArmTimeError", variableSet);

    }

    /// extract the actual variables and write into a variable set
    void extractVariables(RecoTrack& recoTrack)
    {

      m_variables.at(m_prefix + "InOutArmTimeDifference") = recoTrack.getInOutArmTimeDifference();
      m_variables.at(m_prefix + "InOutArmTimeDifferenceError") = recoTrack.getInOutArmTimeDifferenceError();
      m_variables.at(m_prefix + "inGoingArmTime") = recoTrack.getIngoingArmTime();
      m_variables.at(m_prefix + "inGoingArmTimeError") = recoTrack.getIngoingArmTimeError();
      m_variables.at(m_prefix + "outGoingArmTime") = recoTrack.getOutgoingArmTime();
      m_variables.at(m_prefix + "outGoingArmTimeError") = recoTrack.getOutgoingArmTimeError();

      float z0 = -999, d0 = -999, tan_lambda_estimate = -999, omega_estimate = -999, phi0_estimate = -999;
      float d0_variance = -999, z0_variance = -999, phi0_variance = -999, omega_variance = -999, tan_lambda_variance = -999;

      auto genfitTrack = recoTrack.getRelated<Track>("Tracks");
      if (genfitTrack) {
        auto trackFitResult = genfitTrack->getTrackFitResultWithClosestMass(Const::pion);
        if (trackFitResult) {
          z0 = trackFitResult->getZ0();
          d0 = trackFitResult->getD0();
          d0_variance = trackFitResult->getCov()[0];
          z0_variance = trackFitResult->getCov()[12];
          omega_estimate = trackFitResult->getOmega();
          omega_variance = trackFitResult->getCov()[9];
          phi0_variance = trackFitResult->getCov()[5];
          phi0_estimate = trackFitResult->getPhi() > 0.0 ? trackFitResult->getPhi() : trackFitResult->getPhi() + 2.0 * TMath::Pi();

          tan_lambda_estimate = trackFitResult->getCotTheta();
          tan_lambda_variance = trackFitResult->getCov()[14];
          m_variables.at(m_prefix + "first_cdc_layer") = trackFitResult->getHitPatternCDC().getFirstLayer();
          m_variables.at(m_prefix + "last_cdc_layer") = trackFitResult->getHitPatternCDC().getLastLayer();
        }
      }
      // m_variables.at(m_prefix + "z0_estimate") = z0;
      // m_variables.at(m_prefix + "d0_estimate") = d0;
      // m_variables.at(m_prefix + "phi0_estimate") = phi0_estimate;
      // m_variables.at(m_prefix + "omega_estimate") = omega_estimate;
      //  m_variables.at(m_prefix + "tan_lambda_estimate") = tan_lambda_estimate;

      // m_variables.at(m_prefix + "z0_variance") = z0_variance;
      // m_variables.at(m_prefix + "d0_variance") = d0_variance;
      // m_variables.at(m_prefix + "phi0_variance") = phi0_variance ;
      // m_variables.at(m_prefix + "omega_variance") = omega_variance;
      // m_variables.at(m_prefix + "tan_lambda_variance") = tan_lambda_variance;

      // m_variables.at(m_prefix + "n_hits") = recoTrack.getNumberOfTrackingHits();
      m_variables.at(m_prefix + "n_svd_hits") = recoTrack.getNumberOfSVDHits();
      m_variables.at(m_prefix + "n_cdc_hits") = recoTrack.getNumberOfCDCHits();

      RecoTrack* svdcdc_recoTrack =  recoTrack.getRelated<RecoTrack>("SVDCDCRecoTracks");
      if (svdcdc_recoTrack) {
        auto svdcdc_cov = svdcdc_recoTrack->getSeedCovariance();
        auto svdcdc_mom = svdcdc_recoTrack->getMomentumSeed();
        auto svdcdc_pos = svdcdc_recoTrack->getPositionSeed();
        auto svdcdc_charge_sign = svdcdc_recoTrack->getChargeSeed() > 0 ? 1 : -1;
        auto svdcdc_b_field = BFieldManager::getFieldInTesla(svdcdc_pos).Z();
        const uint16_t svdcdc_NDF = 0xffff;
        auto svdcdc_FitResult = TrackFitResult(svdcdc_pos, svdcdc_mom, svdcdc_cov,
                                               svdcdc_charge_sign, Const::pion, 0, svdcdc_b_field, 0, 0,
                                               svdcdc_NDF);

        m_variables.at(m_prefix + "seed_pz_variance") = svdcdc_cov(5, 5);
        m_variables.at(m_prefix + "seed_pz_estimate") = svdcdc_mom.Z();
        m_variables.at(m_prefix + "seed_z_estimate") = svdcdc_pos.Z() ;
        m_variables.at(m_prefix + "seed_tan_lambda_estimate") = svdcdc_FitResult.getCotTheta();

        float seed_pt_estimate = svdcdc_mom.Rho();
        float seed_pt_variance = (pow(svdcdc_mom.X(), 2) * svdcdc_cov(3, 3) + pow(svdcdc_mom.Y(), 2) * svdcdc_cov(4,
                                  4) - 2 * svdcdc_mom.X() * svdcdc_mom.Y() * svdcdc_cov(3, 4)) / svdcdc_mom.Perp2();
        float seed_pt_resolution = seed_pt_variance / seed_pt_estimate;

        m_variables.at(m_prefix + "seed_pt_estimate") = seed_pt_estimate;
        m_variables.at(m_prefix + "seed_x_estimate") = svdcdc_pos.X();
        m_variables.at(m_prefix + "seed_y_estimate") = svdcdc_pos.Y();
        // m_variables.at(m_prefix + "seed_pt_resolution") = seed_pt_resolution;
        m_variables.at(m_prefix + "seed_py_variance") = svdcdc_cov(4, 4);
        m_variables.at(m_prefix + "seed_d0_estimate") = svdcdc_FitResult.getD0();
        m_variables.at(m_prefix + "seed_omega_variance") = svdcdc_FitResult.getCov()[9];
        m_variables.at(m_prefix + "seed_tan_lambda_variance") = svdcdc_FitResult.getCov()[14];
        m_variables.at(m_prefix + "seed_z_variance") = svdcdc_cov(2, 2);

      }

      //Belle2::RecoTrack*  cdc_track_cand = recoTrack.getRelated<RecoTrack>("CDCRecoTracks");
      //if (cdc_track_cand) {
      // m_variables.at(m_prefix + "cdc_qualityindicator") = cdc_track_cand->getQualityIndicator();
      //}
      for (auto* svdHit : recoTrack.getSVDHitList()) {
        if (svdHit->getSensorID().getLayerNumber() == 3) {
          m_variables.at(m_prefix + "svd_layer3_positionSigma") = svdHit->getPositionSigma();
        }
        if (svdHit->getSensorID().getLayerNumber() == 6) {
          m_variables.at(m_prefix + "svd_layer6_clsTime") = svdHit->getClsTime();
        }
      }
    }


  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  };

}
