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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <limits>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class EventInfoExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit EventInfoExtractor(std::vector<Named<float*>>& variableSet):
      VariableExtractor()
    {
      addVariable("__experiment__", variableSet);
      addVariable("__run__", variableSet);
      addVariable("__event__", variableSet);
      addVariable("N_RecoTracks", variableSet);
      addVariable("N_PXDRecoTracks", variableSet);
      addVariable("N_SVDRecoTracks", variableSet);
      addVariable("N_CDCRecoTracks", variableSet);
      addVariable("N_diff_PXD_SVD_RecoTracks", variableSet);
      addVariable("N_diff_SVD_CDC_RecoTracks", variableSet);
      addVariable("RTs_Min_Mom_diff_Mag", variableSet);
      addVariable("RTs_Min_Mom_diff_Pt", variableSet);
      addVariable("RTs_Min_Pos_diff_Theta", variableSet);
      addVariable("RTs_Min_Pos_diff_Phi", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(const StoreArray<RecoTrack>& recoTracks, const RecoTrack& thisRecoTrack)
    {
      m_variables.at("N_RecoTracks") = recoTracks.getEntries();

      int n_pxdRecoTracks = 0;
      int n_svdRecoTracks = 0;
      int n_cdcRecoTracks = 0;
      float min_mom_diff_mag = std::numeric_limits<float>::max();
      float min_mom_diff_Pt = std::numeric_limits<float>::max();
      float min_pos_diff_Theta = std::numeric_limits<float>::max();
      float min_pos_diff_Phi = std::numeric_limits<float>::max();

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
          min_mom_diff_mag = mom_diff_mag;
        }
        float mom_diff_Pt = fabs(recoTrack.getMomentumSeed().Pt() - thisRecoTrack.getMomentumSeed().Pt());
        if (mom_diff_Pt < min_mom_diff_Pt) {
          min_mom_diff_Pt = mom_diff_Pt;
        }
        float pos_diff_Theta = fabs(recoTrack.getPositionSeed().Theta() - thisRecoTrack.getPositionSeed().Theta());
        if (pos_diff_Theta < min_pos_diff_Theta) {
          min_pos_diff_Theta = pos_diff_Theta;
        }
        float pos_diff_Phi = fabs(recoTrack.getPositionSeed().Phi() - thisRecoTrack.getPositionSeed().Phi());
        if (pos_diff_Phi < min_pos_diff_Phi) {
          min_pos_diff_Phi = pos_diff_Phi;
        }
      }

      StoreObjPtr<EventMetaData> m_eventMetaData;

      m_variables.at("__experiment__") = m_eventMetaData->getExperiment();
      m_variables.at("__run__") = m_eventMetaData->getRun();
      m_variables.at("__event__") = m_eventMetaData->getEvent();
      m_variables.at("N_PXDRecoTracks") = n_pxdRecoTracks;
      m_variables.at("N_SVDRecoTracks") = n_svdRecoTracks;
      m_variables.at("N_CDCRecoTracks") = n_cdcRecoTracks;
      m_variables.at("N_diff_PXD_SVD_RecoTracks") = n_svdRecoTracks - n_pxdRecoTracks;
      m_variables.at("N_diff_SVD_CDC_RecoTracks") = n_cdcRecoTracks - n_svdRecoTracks;
      m_variables.at("RTs_Min_Mom_diff_Mag") = min_mom_diff_mag;
      m_variables.at("RTs_Min_Mom_diff_Pt") = min_mom_diff_Pt;
      m_variables.at("RTs_Min_Pos_diff_Theta") = min_pos_diff_Theta;
      m_variables.at("RTs_Min_Pos_diff_Phi") = min_pos_diff_Phi;
    }

  private:

  };
}
