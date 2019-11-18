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

#include <genfit/MeasuredStateOnPlane.h>
#include <root/TVector3.h>
#include <root/TMatrixDSym.h>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class SubRecoTrackExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit SubRecoTrackExtractor(std::vector<Named<float*>>& variableSet):
      VariableExtractor()
    {
      addVariable("CDC_QI", variableSet);
      addVariable("SVD_QI", variableSet);
      addVariable("PXD_QI", variableSet);

      addVariable("SVD_has_SPTC", variableSet);
      addVariable("SVD_FitSuccessful", variableSet);
      addVariable("CDC_FitSuccessful", variableSet);

      initializeStats("SVD_CDC_CDCwall_Pos", variableSet);
      initializeStats("SVD_CDC_CDCwall_Mom", variableSet);
      initializeStats("SVD_CDC_POCA_Pos", variableSet);
      initializeStats("SVD_CDC_POCA_Mom", variableSet);

      addVariable("SVD_CDC_CDCwall_Chi2", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(RecoTrack const* CDCRecoTrack,
                          RecoTrack const* SVDRecoTrack,
                          RecoTrack const* PXDRecoTrack)
    {
      if (PXDRecoTrack) {
        const float pxdQI = PXDRecoTrack->getQualityIndicator();
        m_variables.at("PXD_QI") = isnan(pxdQI) ? 0. : pxdQI;
      } else {
        m_variables.at("PXD_QI") = -1.;
      }

      if (CDCRecoTrack) {
        const float cdcQI = CDCRecoTrack->getQualityIndicator();
        m_variables.at("CDC_QI") = isnan(cdcQI) ? 0. : cdcQI;
        m_variables.at("CDC_FitSuccessful") = CDCRecoTrack->wasFitSuccessful();
      } else {
        m_variables.at("CDC_QI") = -1.;
        m_variables.at("CDC_FitSuccessful") = -1.;
      }

      if (SVDRecoTrack) {
        m_variables.at("SVD_QI") = SVDRecoTrack->getQualityIndicator();
        m_variables.at("SVD_FitSuccessful") = SVDRecoTrack->wasFitSuccessful();
        m_variables.at("SVD_has_SPTC") = bool(SVDRecoTrack->getRelatedTo<SpacePointTrackCand>("SPTrackCands"));
      } else {
        m_variables.at("SVD_QI") = -1.;
        m_variables.at("SVD_has_SPTC") = -1.;
        m_variables.at("SVD_FitSuccessful") = -1.;
      }

      if (SVDRecoTrack and CDCRecoTrack and SVDRecoTrack->wasFitSuccessful() and CDCRecoTrack->wasFitSuccessful()) {
        extractVariablesAtExtrapolationToCDCWall(CDCRecoTrack, SVDRecoTrack);
        extractVariablesAtExtrapolationToPOCA(CDCRecoTrack, SVDRecoTrack);
      } else {
        setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Pos", nullptr, nullptr);
        setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Mom", nullptr, nullptr);
        m_variables.at("SVD_CDC_CDCwall_Chi2") = -1.;
        setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Pos", nullptr, nullptr);
        setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Mom", nullptr, nullptr);
      }

    }

  protected:
    /// initialize statistics subsets of variables from clusters that get combined for SPTC
    void initializeStats(const std::string& prefix, std::vector<Named<float*>>& variables)
    {
      addVariable(prefix + "_diff_Z", variables);
      addVariable(prefix + "_diff_Pt", variables);
      addVariable(prefix + "_diff_Theta", variables);
      addVariable(prefix + "_diff_Phi", variables);
      addVariable(prefix + "_diff_Mag", variables);
      addVariable(prefix + "_diff_Eta", variables);
    }

    /// calculated differences and saves them in variable set
    void setCDCSVDTrackDifferenceVariables(const std::string& prefix,
                                           const TVector3* svdTrackVector,
                                           const TVector3* cdcTrackVector)
    {
      if (not(svdTrackVector and cdcTrackVector)) {
        m_variables.at(prefix + "_diff_Z") = -1.;
        m_variables.at(prefix + "_diff_Pt") = -1.;
        m_variables.at(prefix + "_diff_Theta") = -1.;
        m_variables.at(prefix + "_diff_Phi") = -1.;
        m_variables.at(prefix + "_diff_Mag") = -1.;
        m_variables.at(prefix + "_diff_Eta") = -1.;
        return;
      }
      m_variables.at(prefix + "_diff_Z") = fabs(cdcTrackVector->Z() - svdTrackVector->Z());
      m_variables.at(prefix + "_diff_Pt") = fabs(cdcTrackVector->Pt() - svdTrackVector->Pt());
      m_variables.at(prefix + "_diff_Theta") = fabs(cdcTrackVector->Theta() - svdTrackVector->Theta());
      m_variables.at(prefix + "_diff_Phi") = fabs(cdcTrackVector->Phi() - svdTrackVector->Phi());
      m_variables.at(prefix + "_diff_Mag") = fabs(cdcTrackVector->Mag() - svdTrackVector->Mag());
      m_variables.at(prefix + "_diff_Eta") = fabs(cdcTrackVector->Eta() - svdTrackVector->Eta());
    }

    /** Extrapolate fitted RecoTracks from CDC standalone and VXDTF2 tracking to the CDC wall
     * and extract the difference variables there.
     */
    void extractVariablesAtExtrapolationToCDCWall(RecoTrack const* CDCRecoTrack, RecoTrack const* SVDRecoTrack)
    {
      // position and momentum used for extrapolations to the CDC Wall
      TVector3 center(0., 0., 0.);
      TVector3 direction(0., 0., 1.);

      genfit::MeasuredStateOnPlane svdTrackStateOnPlane;
      genfit::MeasuredStateOnPlane cdcTrackStateOnPlane;
      try {
        // do extrapolation of SVD and CDC onto CDCwall
        svdTrackStateOnPlane = SVDRecoTrack->getMeasuredStateOnPlaneFromLastHit();
        cdcTrackStateOnPlane = CDCRecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdcTrackStateOnPlane.extrapolateToCylinder(m_CDC_wall_radius, center, direction);
        svdTrackStateOnPlane.extrapolateToPlane(cdcTrackStateOnPlane.getPlane());
      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: SVDRecoTrack and/or CDCRecoTrack extrapolation to the CDCwall failed!\n"
                  << "-->" << e.what());
        setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Pos", nullptr, nullptr);
        setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Mom", nullptr, nullptr);
        m_variables.at("SVD_CDC_CDCwall_Chi2") = -1.;
        return;
      }

      // set differences in Position and Momentum
      const TVector3 svdTrackPositionOnPlaneAtCDCWall = svdTrackStateOnPlane.getPos();
      const TVector3 svdTrackMomentumOnPlaneAtCDCWall = svdTrackStateOnPlane.getMom();
      const TVector3 cdcTrackPositionOnPlaneAtCDCWall = cdcTrackStateOnPlane.getPos();
      const TVector3 cdcTrackMomentumOnPlaneAtCDCWall = cdcTrackStateOnPlane.getMom();
      setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Pos", &svdTrackPositionOnPlaneAtCDCWall, &cdcTrackPositionOnPlaneAtCDCWall);
      setCDCSVDTrackDifferenceVariables("SVD_CDC_CDCwall_Mom", &svdTrackMomentumOnPlaneAtCDCWall, &cdcTrackMomentumOnPlaneAtCDCWall);
      try {
        // calculate chi2 between SVD and CDC (like in VXDCDCTrackMerger)
        const TMatrixDSym invCovariance = (svdTrackStateOnPlane.getCov() + cdcTrackStateOnPlane.getCov()).Invert();
        TVectorD stateDifference = cdcTrackStateOnPlane.getState() - svdTrackStateOnPlane.getState();
        stateDifference *= invCovariance;
        const float chi2 = stateDifference * (cdcTrackStateOnPlane.getState() - svdTrackStateOnPlane.getState());
        m_variables.at("SVD_CDC_CDCwall_Chi2") = chi2;

      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: Matrix is singular!\n"
                  << "-->" << e.what());
        m_variables.at("SVD_CDC_CDCwall_Chi2") = -1.;
        return;
      }
    }

    /** Extrapolate fitted RecoTracks from CDC standalone and VXDTF2 tracking to the
     * POCA and extract the difference variables there.
     */
    void extractVariablesAtExtrapolationToPOCA(RecoTrack const* CDCRecoTrack, RecoTrack const* SVDRecoTrack)
    {
      // position and momentum used for extrapolations to the CDC Wall
      const TVector3 linePoint(0., 0., 0.);
      const TVector3 lineDirection(0., 0., 1.);

      genfit::MeasuredStateOnPlane svdTrackStateOnPlane;
      genfit::MeasuredStateOnPlane cdcTrackStateOnPlane;
      try {
        // do extrapolation of SVD and CDC onto CDCwall
        svdTrackStateOnPlane = SVDRecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdcTrackStateOnPlane = CDCRecoTrack->getMeasuredStateOnPlaneFromFirstHit();
        cdcTrackStateOnPlane.extrapolateToLine(linePoint, lineDirection);
        svdTrackStateOnPlane.extrapolateToLine(linePoint, lineDirection);
      } catch (genfit::Exception const& e) {
        // extrapolation not possible, skip this track
        B2WARNING("SubRecoTrackExtractor: SVDRecoTrack and/or CDCRecoTrack extrapolation to POCA failed!\n"
                  << "-->" << e.what());
        setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Pos", nullptr, nullptr);
        setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Mom", nullptr, nullptr);
        return;
      }

      // set differences in Position and Momentum
      const TVector3 svdTrackPositionOnPlaneAtPOCA = svdTrackStateOnPlane.getPos();
      const TVector3 svdTrackMomentumOnPlaneAtPOCA = svdTrackStateOnPlane.getMom();
      const TVector3 cdcTrackPositionOnPlaneAtPOCA = cdcTrackStateOnPlane.getPos();
      const TVector3 cdcTrackMomentumOnPlaneAtPOCA = cdcTrackStateOnPlane.getMom();
      setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Pos", &svdTrackPositionOnPlaneAtPOCA, &cdcTrackPositionOnPlaneAtPOCA);
      setCDCSVDTrackDifferenceVariables("SVD_CDC_POCA_Mom", &svdTrackMomentumOnPlaneAtPOCA, &cdcTrackMomentumOnPlaneAtPOCA);
    }

  private:
    /** Radius of the inner CDC wall in centimeters */
    const double m_CDC_wall_radius = 16.25;
  };
}
