/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - 2018 - Belle II Collaboration                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/eventTimeExtraction/utilities/TimeExtractionUtils.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>

#include <genfit/Tools.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <framework/logging/LogMethod.h>
#include <TDecompChol.h>

using namespace Belle2;


namespace {
  /** TMatrixTSym::SetSub is sloooooowwwww as it uses
      TMatrixT::operator() which does bounds checking.  Hence this
      homegrown alternative.  */
  template<typename T>
  void setSubOnDiagonal(TMatrixTSym <T>& target, int iRow,
                        const TMatrixTSym <T>& source)
  {
    const int nColsTarget = target.GetNrows();
    const int nColsSource = source.GetNrows();

    T* pt = target.GetMatrixArray();
    const T* ps = source.GetMatrixArray();

    for (int i = 0; i < nColsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iRow + i) * nColsTarget + (iRow + j)] = ps[i * nColsSource + j];
      }
    }
  }

  /** Insert a TMatrixT somewhere off-diagonal into a TMatrixTSym.
      This takes care of the mirrored part.
      So the source matrix is inserted starting from the (iRow, iCol) element
      and the transposed               starting from the (iCol, iRow) element  */
  template<typename T>
  void setSubOffDiagonal(TMatrixTSym <T>& target, int iRow, int iCol,
                         const TMatrixT <T>& source)
  {
    const int nColsTarget = target.GetNcols();
    const int nRowsSource = source.GetNrows();
    const int nColsSource = source.GetNcols();

    T* pt = target.GetMatrixArray();
    const T* ps = source.GetMatrixArray();

    for (int i = 0; i < nRowsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iRow + i) * nColsTarget + (iCol + j)] = ps[i * nColsSource + j];
      }
    }
    for (int i = 0; i < nRowsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iCol + j) * nColsTarget + (iRow + i)] = ps[i * nColsSource + j];
      }
    }
  }
}


void TimeExtractionUtils::addEventT0WithQuality(std::vector<RecoTrack*>& recoTracks,
                                                const StoreObjPtr<EventT0>& eventT0,
                                                std::vector<std::pair<EventT0::EventT0Component, double>>& eventT0WithQualityIndex)
{
  if (not eventT0->hasEventT0()) {
    B2DEBUG(50, "No event t0 is set. Not testing.");
    return;
  }

  // As we do not know what happened before, we have to set the dirty flag here to force a refit.
  const auto& chi2_with_ndf = TimeExtractionUtils::getChi2WithFit(recoTracks, true);
  const double quality = chi2_with_ndf.second;

  if (std::isnan(quality)) {
    B2DEBUG(50, "The calculated quality is nan. Not using this EventT0 of " << eventT0->getEventT0());
    return;
  }

  B2DEBUG(50, "The iteration gave a result of " << quality << " for " << eventT0->getEventT0());
  eventT0WithQualityIndex.emplace_back(*eventT0->getEventT0Component(), quality);
}


std::pair<double, double> TimeExtractionUtils::getChi2WithFit(const std::vector<RecoTrack*>& recoTracks, bool setDirtyFlag)
{
  TrackFitter trackFitter;

  double summedChi2 = 0;
  double summedNDF = 0;
  unsigned int numberOfFittableRecoTracks = 0;

  for (RecoTrack* recoTrack : recoTracks) {
    if (setDirtyFlag) {
      recoTrack->setDirtyFlag();
    }

    if (not trackFitter.fit(*recoTrack)) {
      continue;
    }

    const double chi2 = TimeExtractionUtils::extractReducedChi2(*recoTrack);
    const double ndf = recoTrack->getTrackFitStatus()->getNdf();

    if (std::isnan(chi2)) {
      continue;
    }

    numberOfFittableRecoTracks++;
    summedChi2 += chi2;
    summedNDF += ndf;
  }

  if (numberOfFittableRecoTracks == 0) {
    return {NAN, NAN};
  }

  return {summedChi2 / numberOfFittableRecoTracks, summedNDF / numberOfFittableRecoTracks};
}


std::pair<double, double> TimeExtractionUtils::getExtractedTimeAndUncertaintyWithFit(const std::vector<RecoTrack*>& recoTracks,
    bool setDirtyFlag)
{
  TrackFitter trackFitter;

  double sumFirstDerivatives = 0;
  double sumSecondDerivatives = 0;
  unsigned int numberOfFittableRecoTracks = 0;

  for (RecoTrack* recoTrack : recoTracks) {
    if (setDirtyFlag) {
      recoTrack->setDirtyFlag();
    }

    if (not trackFitter.fit(*recoTrack)) {
      continue;
    }

    const auto& chi2Derivatives = TimeExtractionUtils::getChi2Derivatives(*recoTrack);
    const double dchi2da = chi2Derivatives.first;
    const double d2chi2da2 = chi2Derivatives.second;

    if (std::isnan(dchi2da) or std::isnan(d2chi2da2)) {
      continue;
    }

    if (d2chi2da2 > 20) {
      B2DEBUG(50, "Track with bad second derivative");
      continue;
    }

    numberOfFittableRecoTracks++;
    sumFirstDerivatives += dchi2da;
    sumSecondDerivatives += d2chi2da2;
  }

  if (numberOfFittableRecoTracks == 0) {
    return {NAN, NAN};
  }

  const double extractedEventT0 = sumFirstDerivatives / sumSecondDerivatives;
  const double extractedEventT0Uncertainty = std::sqrt(2 / sumSecondDerivatives);

  return {extractedEventT0, extractedEventT0Uncertainty};
}

double TimeExtractionUtils::extractReducedChi2(const RecoTrack& recoTrack)
{
  const double chi2 = recoTrack.getTrackFitStatus()->getChi2();
  const double ndf = recoTrack.getTrackFitStatus()->getNdf();

  if (ndf == 0) {
    return NAN;
  }

  return chi2 / ndf;
}


std::pair<double, double> TimeExtractionUtils::getChi2Derivatives(const RecoTrack& recoTrack)
{
  // Check if track is ready to be used.
  if (recoTrack.getNumberOfCDCHits() == 0) {
    B2DEBUG(200, "No CDC hits in track.");
    return {NAN, NAN};
  }

  if (recoTrack.wasFitSuccessful()) {
    const genfit::FitStatus* fs = recoTrack.getTrackFitStatus();
    if (!fs)
      return {NAN, NAN};
    if (fs->isTrackPruned()) {
      B2WARNING("Skipping pruned track");
      return {NAN, NAN};
    }
  }

  try {
    const std::vector<int>& vDimMeas = TimeExtractionUtils::getMeasurementDimensions(recoTrack);

    TMatrixDSym fullCovariance;
    bool success = buildFullCovarianceMatrix(recoTrack, fullCovariance);
    if (!success) {
      // Error printed inside.
      return {NAN, NAN};
    }
    TMatrixDSym fullResidualCovariance;
    TMatrixDSym inverseFullMeasurementCovariance;
    buildFullResidualCovarianceMatrix(recoTrack, vDimMeas, fullCovariance, fullResidualCovariance,
                                      inverseFullMeasurementCovariance);

    TVectorD residuals;
    TVectorD residualsTimeDerivative;
    buildResidualsAndTimeDerivative(recoTrack, vDimMeas, residuals, residualsTimeDerivative);

    // Equations with their numbers from 0810.2241:
    // 2 At V^-1 (V - HCHt) V^-1 r       (9)
    // = 2 At V^-1 r for fitted tracks   (12)
    const double dchi2da = 2. * residualsTimeDerivative * (inverseFullMeasurementCovariance * residuals);

    // (2 At V^-1 (V - HCHt) V^-1 A)^-1, note that this should be   (10)
    // SimilarityT(...) if (...) were a matrix.
    const double d2chi2da2 = 2. * fullResidualCovariance.Similarity(inverseFullMeasurementCovariance * residualsTimeDerivative);


    if (d2chi2da2 > 20) {
      B2DEBUG(200, "Track with bad second derivative");
      return {NAN, NAN};
    }
    return {dchi2da, d2chi2da2};
  } catch (...) {
    B2DEBUG(50, "Failed time extraction - skipping track");
    return {NAN, NAN};
  }
}


std::vector<int> TimeExtractionUtils::getMeasurementDimensions(const RecoTrack& recoTrack)
{
  const auto& hitPoints = recoTrack.getHitPointsWithMeasurement();

  std::vector<int> vDimMeas;
  vDimMeas.reserve(hitPoints.size());

  for (const auto& hit : hitPoints) {
    vDimMeas.push_back(hit->getRawMeasurement(0)->getDim());
  }

  return vDimMeas;
}


bool TimeExtractionUtils::buildFullCovarianceMatrix(const RecoTrack& recoTrack,
                                                    TMatrixDSym& fullCovariance)
{
  const auto* kfs = dynamic_cast<const genfit::KalmanFitStatus*>(recoTrack.getTrackFitStatus());

  if (!kfs) {
    B2ERROR("Track not fitted with a Kalman fitter.");
    return false;
  }

  if (!kfs->isFitConverged()) {
    B2ERROR("Track fit didn't converge.");
    return false;
  }

  if (!kfs->isFittedWithReferenceTrack()) {
    B2ERROR("No reference track.");
    return false;
  }

  const auto& hitPoints = recoTrack.getHitPointsWithMeasurement();
  const unsigned int nPoints = hitPoints.size();
  const genfit::AbsTrackRep* rep = recoTrack.getCardinalRepresentation();
  const int nDim = rep->getDim();

  fullCovariance.ResizeTo(nPoints * nDim, nPoints * nDim);
  std::vector<TMatrixD> vFitterGain;
  vFitterGain.reserve(nPoints);
  for (unsigned int i = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = hitPoints[i];
    const genfit::KalmanFitterInfo* fi = tp->getKalmanFitterInfo();
    if (!fi) {
      B2DEBUG(50, "Missing KalmanFitterInfo - skipping track");
      return false;
    }

    // Diagonal part of the full covariance matrix are the covariances
    // of the smoothed states.
    const genfit::MeasuredStateOnPlane& mop = fi->getFittedState();
    setSubOnDiagonal(fullCovariance, i * nDim, mop.getCov());

    // Build the corresponding smoother gain matrix.
    if (i + 1 < nPoints) {
      const genfit::TrackPoint* tpNext = hitPoints[i + 1];
      const genfit::KalmanFitterInfo* fiNext = tpNext->getKalmanFitterInfo();
      if (!fiNext) {
        B2DEBUG(50, "Missing next KalmanFitterInfo - skipping track");
        return false;
      }

      // update at i
      const genfit::MeasuredStateOnPlane* update = fi->getForwardUpdate();
      // transport to i+1 prediction at i+1
      const genfit::ReferenceStateOnPlane* rsop = fiNext->getReferenceState();
      if (rsop) {
        const genfit::MeasuredStateOnPlane* pred = fiNext->getForwardPrediction();

        // Evaluate (C^i_i+1)^-1 F_i
        TDecompChol decomp(pred->getCov());
        TMatrixD F = rsop->getForwardTransportMatrix();
        decomp.MultiSolve(F);

        // Calculate gain matrix as
        //   C_i F_i^T (C^i_i+1)^-1 = C_i ((C^i_i+1)^-1 F_i)^T
        // in the notation of 0810.2241
        vFitterGain.emplace_back(TMatrixD(update->getCov(),
                                          TMatrixD::kMultTranspose, F));
      } else {
        B2DEBUG(150, "No reference state, substituting empty fitter gain matrix.");
        vFitterGain.emplace_back(TMatrixD(5, 5));
      }
    }

    // Build the off-diagonal elements.
    TMatrixD offDiag = mop.getCov();
    for (int j = i - 1; j >= 0; --j) {
      offDiag = TMatrixD(vFitterGain[j], TMatrixD::kMult, offDiag);
      setSubOffDiagonal(fullCovariance, j * nDim, i * nDim, offDiag);
    }
  }

  return true;
}

bool TimeExtractionUtils::buildFullResidualCovarianceMatrix(const RecoTrack& recoTrack,
                                                            const std::vector<int>& vDimMeas,
                                                            const TMatrixDSym& fullCovariance,
                                                            TMatrixDSym& fullResidualCovariance,
                                                            TMatrixDSym& inverseFullMeasurementCovariance)
{
  const auto& hitPoints = recoTrack.getHitPointsWithMeasurement();
  const unsigned int nPoints = hitPoints.size();
  const genfit::AbsTrackRep* rep = recoTrack.getCardinalRepresentation();
  const int nDim = rep->getDim();
  int measurementDimensions = std::accumulate(vDimMeas.begin(), vDimMeas.end(), 0);
  fullResidualCovariance.ResizeTo(measurementDimensions, measurementDimensions);
  inverseFullMeasurementCovariance.ResizeTo(measurementDimensions, measurementDimensions);

  // Put together the parts containing the track covariances.
  std::vector<TMatrixD> HMatrices;
  HMatrices.reserve(nPoints);
  for (unsigned int i = 0, index = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = hitPoints[i];
    const genfit::AbsMeasurement* meas = tp->getRawMeasurement(0);
    std::unique_ptr<const genfit::AbsHMatrix> pH(meas->constructHMatrix(rep));
    const TMatrixD& H = pH->getMatrix();
    int nDimMeas = vDimMeas[i];
    TMatrixDSym cov = fullCovariance.GetSub(i * nDim, (i + 1) * nDim - 1,
                                            i * nDim, (i + 1) * nDim - 1);
    pH->HMHt(cov);
    setSubOnDiagonal(fullResidualCovariance, index, cov);

    int indexOffDiag = index;
    for (int j = i - 1; j >= 0; --j) {
      int nDimMeasJ = HMatrices[j].GetNrows();
      indexOffDiag -= nDimMeasJ;

      TMatrixD offDiag(HMatrices[j], TMatrixD::kMult,
                       TMatrixD(fullCovariance.GetSub(nDim * j, nDim * (j + 1) - 1,
                                                      nDim * i, nDim * (i + 1) - 1),
                                TMatrixD::kMultTranspose, H));

      setSubOffDiagonal(fullResidualCovariance, indexOffDiag, index, offDiag);
    }

    index += nDimMeas;
    HMatrices.push_back(H);
  }

  // Add the measurment covariances, also calculate their full
  // (block-diagonal) inverse covariance matrix.
  fullResidualCovariance *= -1;
  inverseFullMeasurementCovariance = 0;
  for (unsigned int i = 0, index = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = hitPoints[i];
    const genfit::KalmanFitterInfo* fi = tp->getKalmanFitterInfo();
    const genfit::MeasurementOnPlane& mop = fi->getAvgWeightedMeasurementOnPlane();
    TMatrixDSym cov = mop.getCov();
    const int dim = cov.GetNrows();
    setSubOnDiagonal(fullResidualCovariance, index,
                     cov + fullResidualCovariance.GetSub(index, index + dim - 1,
                                                         index, index + dim - 1));

    genfit::tools::invertMatrix(cov);
    setSubOnDiagonal(inverseFullMeasurementCovariance, index, cov);

    index += dim;
  }

  return true;
}


void TimeExtractionUtils::buildResidualsAndTimeDerivative(const RecoTrack& recoTrack,
                                                          const std::vector<int>& vDimMeas,
                                                          TVectorD& residuals,
                                                          TVectorD& residualTimeDerivative)
{
  // The residuals and their derivatives WRT the event time.

  int measurementDimensions = std::accumulate(vDimMeas.begin(), vDimMeas.end(), 0);
  residuals.ResizeTo(measurementDimensions);
  residualTimeDerivative.ResizeTo(measurementDimensions);

  const auto& hitPoints = recoTrack.getHitPointsWithMeasurement();
  const unsigned int nPoints = hitPoints.size();
  for (unsigned int i = 0, index = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = hitPoints[i];
    const genfit::KalmanFitterInfo* fi = tp->getKalmanFitterInfo();

    const std::vector<double>& weights = fi->getWeights();
    TVectorD weightedResidual(vDimMeas[i]);
    for (size_t iMeas = 0; iMeas < fi->getNumMeasurements(); ++iMeas) {
      weightedResidual += weights[iMeas] * fi->getResidual(iMeas).getState();
    }
    residuals.SetSub(index, weightedResidual);
    if (dynamic_cast<const CDCRecoHit*>(tp->getRawMeasurement(0))) {
      const CDCRecoHit* hit = static_cast<const CDCRecoHit*>(tp->getRawMeasurement(0));
      std::vector<double> deriv = hit->timeDerivativesMeasurementsOnPlane(fi->getFittedState());

      double weightedDeriv = weights[0] * deriv[0] + weights[1] * deriv[1];
      residualTimeDerivative[index] = weightedDeriv;
    }
    index += vDimMeas[i];
  }
}