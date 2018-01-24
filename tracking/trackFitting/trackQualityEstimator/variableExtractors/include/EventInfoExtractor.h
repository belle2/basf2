/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/dataobjects/RecoTrack.h>
//#include <tracking/dataobjects/RecoHitInformation.h>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class EventInfoExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    EventInfoExtractor(std::vector<Named<float*>>& variableSet):
      VariableExtractor()
    {
      addVariable("N_RecoTracks", variableSet);
//      addVariable("N_PXDRecoTracks", variableSet);
//      addVariable("N_SVDRecoTracks", variableSet);
//      addVariable("N_CDCRecoTracks", variableSet);
      addVariable("N_diff_PXD_SVD_RecoTracks", variableSet);
      addVariable("N_diff_SVD_CDC_RecoTracks", variableSet);

      addVariable("RTs_Min_Mom_diff_Mag", variableSet);
      addVariable("RTs_Min_Mom_diff_Mag_idx", variableSet);
      addVariable("RTs_Min_Mom_diff_Pt", variableSet);
      addVariable("RTs_Min_Mom_diff_Pt_idx", variableSet);

      addVariable("RTs_Min_Pos_diff_Theta", variableSet);
      addVariable("RTs_Min_Pos_diff_Theta_idx", variableSet);
      addVariable("RTs_Min_Pos_diff_Phi", variableSet);
      addVariable("RTs_Min_Pos_diff_Phi_idx", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(const StoreArray<RecoTrack>& recoTracks, const RecoTrack& thisRecoTrack)
    {
      m_variables.at("N_RecoTracks") = recoTracks.getEntries();

      int n_pxdRecoTracks = 0; int n_svdRecoTracks = 0; int n_cdcRecoTracks = 0;
      float min_mom_diff_mag = std::numeric_limits<float>::infinity();   int min_mom_diff_mag_idx = -1;
      float min_mom_diff_Pt = std::numeric_limits<float>::infinity();    int min_mom_diff_Pt_idx = -1;
      float min_pos_diff_Theta = std::numeric_limits<float>::infinity(); int min_pos_diff_Theta_idx = -1;
      float min_pos_diff_Phi = std::numeric_limits<float>::infinity();   int min_pos_diff_Phi_idx = -1;

      for (const RecoTrack& recoTrack : recoTracks) {
        if (recoTrack.hasPXDHits())
          n_pxdRecoTracks++;
        if (recoTrack.hasSVDHits())
          n_svdRecoTracks++;
        if (recoTrack.hasCDCHits())
          n_cdcRecoTracks++;

        if (recoTrack.getArrayIndex() == thisRecoTrack.getArrayIndex())
          continue;

        float mom_diff_mag = fabs(recoTrack.getMomentumSeed().Mag() - thisRecoTrack.getMomentumSeed().Mag());
        if (mom_diff_mag < min_mom_diff_mag) {
          min_mom_diff_mag     = mom_diff_mag;
          min_mom_diff_mag_idx = recoTrack.getArrayIndex();
        }

        float mom_diff_Pt = fabs(recoTrack.getMomentumSeed().Pt() - thisRecoTrack.getMomentumSeed().Pt());
        if (mom_diff_Pt < min_mom_diff_Pt) {
          min_mom_diff_Pt     = mom_diff_Pt;
          min_mom_diff_Pt_idx = recoTrack.getArrayIndex();
        }

        float pos_diff_Theta = fabs(recoTrack.getPositionSeed().Theta() - thisRecoTrack.getPositionSeed().Theta());
        if (pos_diff_Theta < min_pos_diff_Theta) {
          min_pos_diff_Theta     = pos_diff_Theta;
          min_pos_diff_Theta_idx = recoTrack.getArrayIndex();
        }

        float pos_diff_Phi = fabs(recoTrack.getPositionSeed().Phi() - thisRecoTrack.getPositionSeed().Phi());
        if (pos_diff_Phi < min_pos_diff_Phi) {
          min_pos_diff_Phi     = pos_diff_Phi;
          min_pos_diff_Phi_idx = recoTrack.getArrayIndex();
        }

      }

      m_variables.at("N_diff_PXD_SVD_RecoTracks") = n_svdRecoTracks - n_pxdRecoTracks;
      m_variables.at("N_diff_SVD_CDC_RecoTracks") = n_cdcRecoTracks - n_svdRecoTracks;

      m_variables.at("RTs_Min_Mom_diff_Mag")     = min_mom_diff_mag;
      m_variables.at("RTs_Min_Mom_diff_Mag_idx") = min_mom_diff_mag_idx;

      m_variables.at("RTs_Min_Mom_diff_Pt")     = min_mom_diff_Pt;
      m_variables.at("RTs_Min_Mom_diff_Pt_idx") = min_mom_diff_Pt_idx;

      m_variables.at("RTs_Min_Pos_diff_Theta")     = min_pos_diff_Theta;
      m_variables.at("RTs_Min_Pos_diff_Theta_idx") = min_pos_diff_Theta_idx;

      m_variables.at("RTs_Min_Pos_diff_Phi")     = min_pos_diff_Phi;
      m_variables.at("RTs_Min_Pos_diff_Phi_idx") = min_pos_diff_Phi_idx;

    }

  private:

  };
}
