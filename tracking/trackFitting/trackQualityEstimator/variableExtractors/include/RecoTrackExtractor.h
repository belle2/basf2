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
#include <framework/geometry/B2Vector3.h>
#include <limits>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class RecoTrackExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit RecoTrackExtractor(std::vector<Named<float*>>& variableSet, const std::string& prefix = ""):
      VariableExtractor(), m_prefix(prefix)
    {
      addVariable(prefix + "pdg_ID", variableSet);
      addVariable(prefix + "pdg_ID_Mother", variableSet);
      addVariable(prefix + "is_Vzero_Daughter", variableSet);
      addVariable(prefix + "is_Primary", variableSet);

      addVariable(prefix + "z0", variableSet);
      addVariable(prefix + "d0", variableSet);

      addVariable(prefix + "seed_Charge", variableSet);

      addVariable(prefix + "seed_Pos_Pt", variableSet);
      addVariable(prefix + "seed_Pos_Z", variableSet);
      addVariable(prefix + "seed_Pos_Mag", variableSet);
      addVariable(prefix + "seed_Pos_Theta", variableSet);
      addVariable(prefix + "seed_Pos_Phi", variableSet);

      addVariable(prefix + "seed_Mom_Pt", variableSet);
      addVariable(prefix + "seed_Mom_Z", variableSet);
      addVariable(prefix + "seed_Mom_Mag", variableSet);
      addVariable(prefix + "seed_Mom_Theta", variableSet);
      addVariable(prefix + "seed_Mom_Phi", variableSet);

      addVariable(prefix + "seed_Time", variableSet);

      addVariable(prefix + "N_tracking_hits", variableSet);
      addVariable(prefix + "N_CDC_hits", variableSet);
      addVariable(prefix + "N_SVD_hits", variableSet);
      addVariable(prefix + "N_PXD_hits", variableSet);

      addVariable(prefix + "Fit_Charge", variableSet);
      addVariable(prefix + "Fit_Chi2", variableSet);
      addVariable(prefix + "Fit_Ndf", variableSet);
      addVariable(prefix + "Fit_NFailedPoints", variableSet);
      addVariable(prefix + "Fit_PVal", variableSet);
      addVariable(prefix + "Fit_Successful", variableSet);

      addVariable(prefix + "POCA_Pos_Pt", variableSet);
      addVariable(prefix + "POCA_Pos_Z", variableSet);
      addVariable(prefix + "POCA_Pos_Mag", variableSet);
      addVariable(prefix + "POCA_Pos_Theta", variableSet);
      addVariable(prefix + "POCA_Pos_Phi", variableSet);

      addVariable(prefix + "POCA_Mom_Pt", variableSet);
      addVariable(prefix + "POCA_Mom_Z", variableSet);
      addVariable(prefix + "POCA_Mom_Mag", variableSet);
      addVariable(prefix + "POCA_Mom_Theta", variableSet);
      addVariable(prefix + "POCA_Mom_Phi", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(const RecoTrack& recoTrack)
    {
      float pdgID = 0;
      float pdgIDMother = 0;
      float isVzeroDaughter = -1;
      float isPrimary = -1;
      auto mcparticle = recoTrack.getRelated<MCParticle>();
      if (mcparticle) {
        pdgID = static_cast<float>(mcparticle->getPDG());
        pdgIDMother = static_cast<float>(mcparticle->getMother()->getPDG());
        isPrimary = static_cast<float>(mcparticle->isPrimaryParticle());
        if (abs(pdgIDMother) == 310 || abs(pdgIDMother) == 3122) {
          isVzeroDaughter = 1;
        } else {
          isVzeroDaughter = 0;
        }
      }
      float z0 = -999;
      float d0 = -999;
      auto genfitTrack = recoTrack.getRelated<Track>("MDSTTracks");
      if (genfitTrack) {
        auto trackFitResult = genfitTrack->getTrackFitResultWithClosestMass(Const::pion);
        if (trackFitResult) {
          z0 = trackFitResult->getZ0();
          d0 = trackFitResult->getD0();
        }
      }
      m_variables.at(m_prefix + "pdg_ID") = pdgID;
      m_variables.at(m_prefix + "pdg_ID_Mother") = pdgIDMother;
      m_variables.at(m_prefix + "is_Vzero_Daughter") = isVzeroDaughter;
      m_variables.at(m_prefix + "is_Primary") = isPrimary;

      m_variables.at(m_prefix + "z0") = z0;
      m_variables.at(m_prefix + "d0") = d0;

      m_variables.at(m_prefix + "seed_Charge") = recoTrack.getChargeSeed();

      m_variables.at(m_prefix + "seed_Pos_Pt") = recoTrack.getPositionSeed().Rho();
      m_variables.at(m_prefix + "seed_Pos_Z") = recoTrack.getPositionSeed().Z();
      m_variables.at(m_prefix + "seed_Pos_Mag") = recoTrack.getPositionSeed().R();
      m_variables.at(m_prefix + "seed_Pos_Theta") = recoTrack.getPositionSeed().Theta();
      m_variables.at(m_prefix + "seed_Pos_Phi") = recoTrack.getPositionSeed().Phi();

      m_variables.at(m_prefix + "seed_Mom_Pt") = recoTrack.getMomentumSeed().Rho();
      m_variables.at(m_prefix + "seed_Mom_Z") = recoTrack.getMomentumSeed().Z();
      m_variables.at(m_prefix + "seed_Mom_Mag") = recoTrack.getMomentumSeed().R();
      m_variables.at(m_prefix + "seed_Mom_Theta") = recoTrack.getMomentumSeed().Theta();
      m_variables.at(m_prefix + "seed_Mom_Phi") = recoTrack.getMomentumSeed().Phi();

      m_variables.at(m_prefix + "seed_Time") = recoTrack.getTimeSeed();

      m_variables.at(m_prefix + "N_tracking_hits") = recoTrack.getNumberOfTrackingHits();
      m_variables.at(m_prefix + "N_CDC_hits") = recoTrack.getNumberOfCDCHits();
      m_variables.at(m_prefix + "N_SVD_hits") = recoTrack.getNumberOfSVDHits();
      m_variables.at(m_prefix + "N_PXD_hits") = recoTrack.getNumberOfPXDHits();

      const genfit::FitStatus* rt_TrackFitStatus = recoTrack.getTrackFitStatus();
      if (rt_TrackFitStatus) {
        m_variables.at(m_prefix + "Fit_Charge") = rt_TrackFitStatus->getCharge();
        m_variables.at(m_prefix + "Fit_Chi2") = rt_TrackFitStatus->getChi2();
        m_variables.at(m_prefix + "Fit_Ndf") = rt_TrackFitStatus->getNdf();
        m_variables.at(m_prefix + "Fit_NFailedPoints") = rt_TrackFitStatus->getNFailedPoints();
        m_variables.at(m_prefix + "Fit_PVal") = rt_TrackFitStatus->getPVal();
      } else {
        m_variables.at(m_prefix + "Fit_Charge") = -std::numeric_limits<float>::max();
        m_variables.at(m_prefix + "Fit_Chi2") = -1.;
        m_variables.at(m_prefix + "Fit_Ndf") = -1.;
        m_variables.at(m_prefix + "Fit_NFailedPoints") = -1.;
        m_variables.at(m_prefix + "Fit_PVal") = -1.;
      }

      m_variables.at(m_prefix + "Fit_Successful") = (float)recoTrack.wasFitSuccessful();

      if (recoTrack.wasFitSuccessful()) {
        B2Vector3D linePoint(0., 0., 0.);
        B2Vector3D lineDirection(0., 0., 1.);

        genfit::MeasuredStateOnPlane reco_sop;
        try {
          reco_sop = recoTrack.getMeasuredStateOnPlaneFromFirstHit();
          reco_sop.extrapolateToLine(linePoint, lineDirection);
          m_variables.at(m_prefix + "POCA_Pos_Pt") = reco_sop.getPos().Pt();
          m_variables.at(m_prefix + "POCA_Pos_Z") = reco_sop.getPos().Z();
          m_variables.at(m_prefix + "POCA_Pos_Mag") = reco_sop.getPos().Mag();
          m_variables.at(m_prefix + "POCA_Pos_Theta") = reco_sop.getPos().Theta();
          m_variables.at(m_prefix + "POCA_Pos_Phi") = reco_sop.getPos().Phi();

          m_variables.at(m_prefix + "POCA_Mom_Pt") = reco_sop.getMom().Pt();
          m_variables.at(m_prefix + "POCA_Mom_Z") = reco_sop.getMom().Z();
          m_variables.at(m_prefix + "POCA_Mom_Mag") = reco_sop.getMom().Mag();
          m_variables.at(m_prefix + "POCA_Mom_Theta") = reco_sop.getMom().Theta();
          m_variables.at(m_prefix + "POCA_Mom_Phi") = reco_sop.getMom().Phi();
        } catch (genfit::Exception const& e) {
          // extrapolation not possible, skip this track
          B2WARNING("RecoTrackExtractor: recoTrack BeamPipe POCA extrapolation failed!\n"
                    << "-->" << e.what());
          m_variables.at(m_prefix + "POCA_Pos_Pt") = -1;
          m_variables.at(m_prefix + "POCA_Pos_Z") = -std::numeric_limits<float>::max();
          m_variables.at(m_prefix + "POCA_Pos_Mag") = -1;
          m_variables.at(m_prefix + "POCA_Pos_Theta") = -10;
          m_variables.at(m_prefix + "POCA_Pos_Phi") = -10;

          m_variables.at(m_prefix + "POCA_Mom_Pt") = -1;
          m_variables.at(m_prefix + "POCA_Mom_Z") = -std::numeric_limits<float>::max();
          m_variables.at(m_prefix + "POCA_Mom_Mag") = -1;
          m_variables.at(m_prefix + "POCA_Mom_Theta") = -10;
          m_variables.at(m_prefix + "POCA_Mom_Phi") = -10;
        }
      } else {
        m_variables.at(m_prefix + "POCA_Pos_Pt") = -1;
        m_variables.at(m_prefix + "POCA_Pos_Z") = -std::numeric_limits<float>::max();
        m_variables.at(m_prefix + "POCA_Pos_Mag") = -1;
        m_variables.at(m_prefix + "POCA_Pos_Theta") = -10;
        m_variables.at(m_prefix + "POCA_Pos_Phi") = -10;

        m_variables.at(m_prefix + "POCA_Mom_Pt") = -1;
        m_variables.at(m_prefix + "POCA_Mom_Z") = -std::numeric_limits<float>::max();
        m_variables.at(m_prefix + "POCA_Mom_Mag") = -1;
        m_variables.at(m_prefix + "POCA_Mom_Theta") = -10;
        m_variables.at(m_prefix + "POCA_Mom_Phi") = -10;
      }

    }

  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  };
}
