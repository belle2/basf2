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
      addVariable(prefix + "seed_pz_estimate", variableSet);
      addVariable(prefix + "seed_pz_variance", variableSet);
      addVariable(prefix + "seed_z_estimate", variableSet);
      addVariable(prefix + "seed_tan_lambda_estimate", variableSet);
      addVariable(prefix + "seed_pt_estimate", variableSet);
      addVariable(prefix + "seed_x_estimate", variableSet);
      addVariable(prefix + "seed_y_estimate", variableSet);
      addVariable(prefix + "seed_py_variance", variableSet);
      addVariable(prefix + "seed_d0_estimate", variableSet);
      addVariable(prefix + "seed_omega_variance", variableSet);
      addVariable(prefix + "svd_layer6_clsTime", variableSet);
      addVariable(prefix + "seed_tan_lambda_variance", variableSet);
      addVariable(prefix + "seed_z_variance", variableSet);
      addVariable(prefix + "n_svd_hits", variableSet);
      addVariable(prefix + "n_cdc_hits", variableSet);
      addVariable(prefix + "svd_layer3_positionSigma", variableSet);
      addVariable(prefix + "first_cdc_layer", variableSet);
      addVariable(prefix + "last_cdc_layer", variableSet);
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

      auto cdc_list = recoTrack.getSortedCDCHitList();
      if (cdc_list.size() > 0) {
        auto first_cdc_list = cdc_list.front();
        auto last_cdc_list = cdc_list.back();
        m_variables.at(m_prefix + "first_cdc_layer") =  first_cdc_list->getICLayer();
        m_variables.at(m_prefix + "last_cdc_layer") =  last_cdc_list->getICLayer();
      }

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

        m_variables.at(m_prefix + "seed_pt_estimate") = seed_pt_estimate;
        m_variables.at(m_prefix + "seed_x_estimate") = svdcdc_pos.X();
        m_variables.at(m_prefix + "seed_y_estimate") = svdcdc_pos.Y();
        m_variables.at(m_prefix + "seed_py_variance") = svdcdc_cov(4, 4);
        m_variables.at(m_prefix + "seed_d0_estimate") = svdcdc_FitResult.getD0();
        m_variables.at(m_prefix + "seed_omega_variance") = svdcdc_FitResult.getCov()[9];
        m_variables.at(m_prefix + "seed_tan_lambda_variance") = svdcdc_FitResult.getCov()[14];
        m_variables.at(m_prefix + "seed_z_variance") = svdcdc_cov(2, 2);

      }

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
