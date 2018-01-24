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
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include "genfit/TrackCand.h"
#include "genfit/Track.h"
#include "genfit/RKTrackRep.h"

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class SubRecoTrackExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    SubRecoTrackExtractor(std::vector<Named<float*>>& variableSet):
      VariableExtractor()
    {
      addVariable("CDC_QI", variableSet);
      addVariable("SVD_QI", variableSet);
      addVariable("PXD_QI", variableSet);

      addVariable("SVD_has_SPTC", variableSet);

      initializeStats("SVD_CDC_CDCwall_Pos", variableSet);
      initializeStats("SVD_CDC_CDCwall_Mom", variableSet);
      initializeStats("SVD_CDC_POCA_Pos", variableSet);
      initializeStats("SVD_CDC_POCA_Mom", variableSet);

      addVariable("SVD_CDC_CDCwall_Chi2", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(RecoTrack const* CDCrecoTrack, RecoTrack const* SVDrecoTrack, RecoTrack const* PXDrecoTrack)
    {
      m_variables.at("CDC_QI") = CDCrecoTrack ? CDCrecoTrack->getQualityIndicator() : NAN;
      m_variables.at("PXD_QI") = PXDrecoTrack ? PXDrecoTrack->getQualityIndicator() : NAN;

      if (SVDrecoTrack) {
        m_variables.at("SVD_QI") = SVDrecoTrack->getQualityIndicator();

        if (SVDrecoTrack->getRelatedTo<SpacePointTrackCand>("SPTrackCands"))
          m_variables.at("SVD_has_SPTC") = 1.;
        else
          m_variables.at("SVD_has_SPTC") = 0.;

      } else {
        m_variables.at("SVD_QI") = NAN;
        m_variables.at("SVD_has_SPTC") = 0;
      }

      if (SVDrecoTrack and CDCrecoTrack and SVDrecoTrack->wasFitSuccessful() and CDCrecoTrack->wasFitSuccessful()) {

        CDCwall_extrapolation(CDCrecoTrack, SVDrecoTrack);
        POCA_extrapolation(CDCrecoTrack, SVDrecoTrack);

      } else {
        setDiffs("SVD_CDC_CDCwall_Pos", NULL, NULL);
        setDiffs("SVD_CDC_CDCwall_Mom", NULL, NULL);
        m_variables.at("SVD_CDC_CDCwall_Chi2") = NAN;

        setDiffs("SVD_CDC_POCA_Pos", NULL, NULL);
        setDiffs("SVD_CDC_POCA_Mom", NULL, NULL);
      }

    }

  protected:
    /// initialize statistics subsets of variables from clusters that get combined for SPTC
    void initializeStats(std::string identifier, std::vector<Named<float*>>& variables)
    {
      addVariable(identifier + "_diff_Z", variables);
      addVariable(identifier + "_diff_Pt", variables);
      addVariable(identifier + "_diff_Theta", variables);
      addVariable(identifier + "_diff_Phi", variables);
      addVariable(identifier + "_diff_Mag", variables);
      addVariable(identifier + "_diff_Eta", variables);
    }

    /// calculated differences and saves them in variable set
    void setDiffs(std::string identifier, const TVector3* svd_vec, const TVector3* cdc_vec)
    {
      if (not(svd_vec and cdc_vec)) {
        m_variables.at(identifier + "_diff_Z") = NAN;
        m_variables.at(identifier + "_diff_Pt") = NAN;
        m_variables.at(identifier + "_diff_Theta") = NAN;
        m_variables.at(identifier + "_diff_Phi") = NAN;
        m_variables.at(identifier + "_diff_Mag") = NAN;
        m_variables.at(identifier + "_diff_Eta") = NAN;
        return;
      }

      m_variables.at(identifier + "_diff_Z") = cdc_vec->Z() - svd_vec->Z();
      m_variables.at(identifier + "_diff_Pt") = cdc_vec->Pt() - svd_vec->Pt();
      m_variables.at(identifier + "_diff_Theta") = cdc_vec->Theta() - svd_vec->Theta();
      m_variables.at(identifier + "_diff_Phi") = cdc_vec->Phi() - svd_vec->Phi();
      m_variables.at(identifier + "_diff_Mag") = cdc_vec->Mag() - svd_vec->Mag();
      m_variables.at(identifier + "_diff_Eta") = cdc_vec->Eta() - svd_vec->Eta();

    }

    void CDCwall_extrapolation(RecoTrack const* CDCrecoTrack, RecoTrack const* SVDrecoTrack)
    {
      // position and momentum used for extrapolations to the CDC Wall
      TVector3 center(0., 0., 0.);
      TVector3 direction(0., 0., 1.);

      genfit::MeasuredStateOnPlane svd_sop; genfit::MeasuredStateOnPlane cdc_sop;
      try {
        // do extrapolation of SVD and CDC onto CDCwall
        svd_sop = SVDrecoTrack->getMeasuredStateOnPlaneFromLastHit();
        cdc_sop = CDCrecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, center, direction);
        svd_sop.extrapolateToPlane(cdc_sop.getPlane());
      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: SVDrecoTrack and/or CDCrecoTrack extrapolation to the CDCwall failed!\n"
                  << "-->" << e.what());
        setDiffs("SVD_CDC_CDCwall_Pos", NULL, NULL);
        setDiffs("SVD_CDC_CDCwall_Mom", NULL, NULL);
        m_variables.at("SVD_CDC_CDCwall_Chi2") = NAN;
        return;
      }

      // set differences in Position and Momentum
      TVector3 svd_pos = svd_sop.getPos(); TVector3 svd_mom = svd_sop.getMom();
      TVector3 cdc_pos = cdc_sop.getPos(); TVector3 cdc_mom = cdc_sop.getMom();
      setDiffs("SVD_CDC_CDCwall_Pos", &svd_pos, &cdc_pos);
      setDiffs("SVD_CDC_CDCwall_Mom", &svd_mom, &cdc_mom);

      try {
        // calculate chi2 between SVD and CDC (like in VXDCDCTrackMerger)
        TMatrixDSym inv_covmtrx = (svd_sop.getCov() + cdc_sop.getCov()).Invert();
        TVectorD state_diff = cdc_sop.getState() - svd_sop.getState();
        state_diff *= inv_covmtrx;
        float chi_2 = state_diff * (cdc_sop.getState() - svd_sop.getState());

        m_variables.at("SVD_CDC_CDCwall_Chi2") = chi_2;

      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: Matrix is singular!\n"
                  << "-->" << e.what());
        m_variables.at("SVD_CDC_CDCwall_Chi2") = NAN;
        return;
      }
    }

    void POCA_extrapolation(RecoTrack const* CDCrecoTrack, RecoTrack const* SVDrecoTrack)
    {
      // position and momentum used for extrapolations to the CDC Wall
      TVector3 linePoint(0., 0., 0.);
      TVector3 lineDirection(0., 0., 1.);

      genfit::MeasuredStateOnPlane svd_sop; genfit::MeasuredStateOnPlane cdc_sop;
      try {
        // do extrapolation of SVD and CDC onto CDCwall
        svd_sop = SVDrecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdc_sop = CDCrecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdc_sop.extrapolateToLine(linePoint, lineDirection);
        svd_sop.extrapolateToLine(linePoint, lineDirection);
      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: SVDrecoTrack and/or CDCrecoTrack extrapolation to POCA failed!\n"
                  << "-->" << e.what());
        setDiffs("SVD_CDC_POCA_Pos", NULL, NULL);
        setDiffs("SVD_CDC_POCA_Mom", NULL, NULL);
        return;
      }

      // set differences in Position and Momentum
      TVector3 svd_pos = svd_sop.getPos(); TVector3 svd_mom = svd_sop.getMom();
      TVector3 cdc_pos = cdc_sop.getPos(); TVector3 cdc_mom = cdc_sop.getMom();
      setDiffs("SVD_CDC_POCA_Pos", &svd_pos, &cdc_pos);
      setDiffs("SVD_CDC_POCA_Mom", &svd_mom, &cdc_mom);
    }

  private:
    /** Radius of the inner CDC wall in centimeters */
    double m_CDC_wall_radius = 16.25;

  };
}
