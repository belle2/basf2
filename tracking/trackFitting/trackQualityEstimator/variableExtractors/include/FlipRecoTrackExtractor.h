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
      // Set all values to NaN to make sure that no values from previous recoTrack are used accidentally
      setDefaultValues();

      const auto& cdcHitList = recoTrack.getSortedCDCHitList();
      const RecoTrack* svdcdcRecoTrack =  recoTrack.getRelated<RecoTrack>("SVDCDCRecoTracks");
      // In case the CDC hit list is empty, or there is no svdcdcRecoTrack, set arbitrary default value and return
      if (not svdcdcRecoTrack) {
        return;
      }

      m_variables.at(m_prefix + "InOutArmTimeDifference") = static_cast<float>(recoTrack.getInOutArmTimeDifference());
      m_variables.at(m_prefix + "InOutArmTimeDifferenceError") = static_cast<float>(recoTrack.getInOutArmTimeDifferenceError());
      m_variables.at(m_prefix + "inGoingArmTime") = static_cast<float>(recoTrack.getIngoingArmTime());
      m_variables.at(m_prefix + "inGoingArmTimeError") = static_cast<float>(recoTrack.getIngoingArmTimeError());
      m_variables.at(m_prefix + "outGoingArmTime") = static_cast<float>(recoTrack.getOutgoingArmTime());
      m_variables.at(m_prefix + "outGoingArmTimeError") = static_cast<float>(recoTrack.getOutgoingArmTimeError());

      if (cdcHitList.size() > 0) {
        m_variables.at(m_prefix + "first_cdc_layer") =  static_cast<float>(cdcHitList.front()->getICLayer());
        m_variables.at(m_prefix + "last_cdc_layer") =  static_cast<float>(cdcHitList.back()->getICLayer());
      }

      m_variables.at(m_prefix + "n_svd_hits") = static_cast<float>(recoTrack.getNumberOfSVDHits());
      m_variables.at(m_prefix + "n_cdc_hits") = static_cast<float>(recoTrack.getNumberOfCDCHits());

      const auto& svdcdcRecoTrackCovariance = svdcdcRecoTrack->getSeedCovariance();
      const auto& svdcdcRecoTrackMomentum = svdcdcRecoTrack->getMomentumSeed();
      const auto& svdcdcRecoTrackPosition = svdcdcRecoTrack->getPositionSeed();
      const float svdcdcRecoTrackChargeSign = svdcdcRecoTrack->getChargeSeed() > 0 ? 1.0f : -1.0f;
      const float bFieldValue = static_cast<float>(BFieldManager::getFieldInTesla(svdcdcRecoTrackPosition).Z());
      const uint16_t svdcdcNDF = 0xffff;
      const auto& svdcdcFitResult = TrackFitResult(svdcdcRecoTrackPosition, svdcdcRecoTrackMomentum, svdcdcRecoTrackCovariance,
                                                   svdcdcRecoTrackChargeSign, Const::pion, 0, bFieldValue, 0, 0,
                                                   svdcdcNDF);

      m_variables.at(m_prefix + "seed_pz_variance") = static_cast<float>(svdcdcRecoTrackCovariance(5, 5));
      m_variables.at(m_prefix + "seed_pz_estimate") = static_cast<float>(svdcdcRecoTrackMomentum.Z());
      m_variables.at(m_prefix + "seed_z_estimate") = static_cast<float>(svdcdcRecoTrackPosition.Z());
      m_variables.at(m_prefix + "seed_tan_lambda_estimate") = static_cast<float>(svdcdcFitResult.getCotTheta());

      m_variables.at(m_prefix + "seed_pt_estimate") = static_cast<float>(svdcdcRecoTrackMomentum.Rho());
      m_variables.at(m_prefix + "seed_x_estimate") = static_cast<float>(svdcdcRecoTrackPosition.X());
      m_variables.at(m_prefix + "seed_y_estimate") = static_cast<float>(svdcdcRecoTrackPosition.Y());
      m_variables.at(m_prefix + "seed_py_variance") = static_cast<float>(svdcdcRecoTrackCovariance(4, 4));
      m_variables.at(m_prefix + "seed_d0_estimate") = static_cast<float>(svdcdcFitResult.getD0());
      m_variables.at(m_prefix + "seed_omega_variance") = static_cast<float>(svdcdcFitResult.getCov()[9]);
      m_variables.at(m_prefix + "seed_tan_lambda_variance") = static_cast<float>(svdcdcFitResult.getCov()[14]);
      m_variables.at(m_prefix + "seed_z_variance") = static_cast<float>(svdcdcRecoTrackCovariance(2, 2));

      for (const auto* svdHit : recoTrack.getSVDHitList()) {
        if (svdHit->getSensorID().getLayerNumber() == 3) {
          m_variables.at(m_prefix + "svd_layer3_positionSigma") = static_cast<float>(svdHit->getPositionSigma());
        }
        if (svdHit->getSensorID().getLayerNumber() == 6) {
          m_variables.at(m_prefix + "svd_layer6_clsTime") = static_cast<float>(svdHit->getClsTime());
        }
      }
    }


  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  private:
    /// Set all variables to default error value
    void setDefaultValues()
    {
      m_variables.at(m_prefix + "InOutArmTimeDifference") = Const::floatNaN;
      m_variables.at(m_prefix + "InOutArmTimeDifferenceError") = Const::floatNaN;
      m_variables.at(m_prefix + "inGoingArmTime") = Const::floatNaN;
      m_variables.at(m_prefix + "inGoingArmTimeError") = Const::floatNaN;
      m_variables.at(m_prefix + "outGoingArmTime") = Const::floatNaN;
      m_variables.at(m_prefix + "outGoingArmTimeError") = Const::floatNaN;
      m_variables.at(m_prefix + "first_cdc_layer") =  Const::floatNaN;
      m_variables.at(m_prefix + "last_cdc_layer") =  Const::floatNaN;
      m_variables.at(m_prefix + "n_svd_hits") = Const::floatNaN;
      m_variables.at(m_prefix + "n_cdc_hits") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_pz_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_pz_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_z_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_tan_lambda_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_pt_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_x_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_y_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_py_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_d0_estimate") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_omega_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_tan_lambda_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "seed_z_variance") = Const::floatNaN;
      m_variables.at(m_prefix + "svd_layer3_positionSigma") = Const::floatNaN;
      m_variables.at(m_prefix + "svd_layer6_clsTime") = Const::floatNaN;
    }
  };

}
