/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* The event time is calculated as one would calculate an alignment
   parameter.  We follow the prescription in arXiv:0810.2241
   [physics.ins-det] to build the full covariance matrix for the
   Kalman-fitted track (DAF counts as Kalman).  We then evaluate the
   change in event time that minimizes the sum of chi^2s of the tracks
   following the same procedure that is decribed in loc.cit.

   Unlike the case of alignment, we only have one free parameter (time),
   and therefore there are no large matrices to invert.  The necessary
   timeshift is calculated from the estimated derivatives of chi2^2 in
   the linear approximation as
                  dchi^2   /d^2chi^2
     delta t = - -------- / -------- .
                    dt   /    dt^2
   Here division replaces a matrix inverse (this is loc.cit. Eq. (8)).

   Time-dependence of the fit result comes from the CDC hits, where a
   later time of particle passage corresponds to a greater drift
   circle.  */

#include "tracking/modules/trackTimeExtraction/TrackTimeExtractionModule.h"

#include <algorithm>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TDecompChol.h>
#include <TDecompSVD.h>

#include "genfit/Tools.h"
#include "genfit/TrackCand.h"
#include "genfit/Track.h"
#include "genfit/FitStatus.h"
#include "genfit/KalmanFitterInfo.h"
#include "genfit/KalmanFitStatus.h"
#include "genfit/MeasuredStateOnPlane.h"

using namespace Belle2;

REG_MODULE(TrackTimeExtraction);

namespace {
  std::map<std::string, TH1*> hTimes;
  std::map<std::string, TH1*> hTimesCorrectHypot;
  std::map<std::string, TH1*> hTimesGoodTracks;
  std::map<std::string, TH1*> hTimesFastTracks;
  std::map<std::string, TH1*> hTimesPerTrack;
  std::map<std::string, TH1*> hTimesPerTrackVsPVal;
  std::map<std::string, TH1*> hDerivative;
  std::map<std::string, TH1*> hWeightedDerivative;
  std::map<std::string, TH1*> hWeightedDerivativeBoth;
  std::map<std::string, TH1*> hXTRelation;
  std::map<std::string, TH1*> hEstimatedTimeErrors;
  std::map<std::string, TH1*> hd2chi2da2;

  std::map<std::string, TTree*> t;
  struct {
    // Per Event data.
    int nTracks;
    float pval[100];
    float ndf[100];
    float px[100], py[100], pz[100], pt[100];
    int parentPDG[100];
    int mcPDG[100];
    int PDG[100];

    double dchi2da[100];
    double d2chi2da2[100];
  } td;
}

TrackTimeExtractionModule::TrackTimeExtractionModule() : HistoModule()
{
  setDescription("Build the full covariance matrix for genfit::Tracks.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("InputTrackColName", m_InTrackColName, "StoreArray containing the genfit::Tracks to process", std::string(""));
  addParam("InputTrackCandColName", m_InTrackCandColName,
           "StoreArray of the genfit::TrackCands corresponding to the genfit::Tracks we are processing", std::string(""));
  addParam("OutputTrackCandColName", m_OutTrackCandColName, "StoreArray where to put the new track candidates with adjusted times",
           std::string("refitCands"));

  addParam("DoHistogramming", m_DoHistogramming, "Whether to generate output histograms", false);
}

TrackTimeExtractionModule::~TrackTimeExtractionModule()
{
}

void TrackTimeExtractionModule::initialize()
{
  if (m_DoHistogramming) {
    REG_HISTOGRAM;
  }
  StoreArray<genfit::TrackCand>::required(m_InTrackCandColName);

  StoreArray<genfit::Track> GFTracks(m_InTrackColName);
  GFTracks.isRequired();

  StoreArray<genfit::TrackCand> newTrackCands(m_OutTrackCandColName);
  newTrackCands.registerInDataStore();
  newTrackCands.registerRelationTo(GFTracks);

  StoreArray<MCParticle>::optional();
}


void TrackTimeExtractionModule::defineHisto()
{
  if (!m_DoHistogramming)
    return;

  hTimes[m_InTrackColName] = new TH1D((std::string("hTimes") + m_InTrackColName).c_str(),
                                      "event time shifts calculated", 15000, -30, 30);
  hTimesCorrectHypot[m_InTrackColName] = new TH1D((std::string("hTimesCorrectHypot") + m_InTrackColName).c_str(),
                                                  "event time shifts calculated", 15000, -30, 30);
  hTimesGoodTracks[m_InTrackColName] = new TH1D((std::string("hTimesGoodTracks") + m_InTrackColName).c_str(),
                                                "event time shifts calculated (good tracks)", 15000, -30, 30);
  hTimesFastTracks[m_InTrackColName] = new TH1D((std::string("hTimesFastTracks") + m_InTrackColName).c_str(),
                                                "event time shifts calculated (fast tracks)", 15000, -30, 30);
  hTimesPerTrack[m_InTrackColName] = new TH1D((std::string("hTimesPerTrack") + m_InTrackColName).c_str(),
                                              "per track time shifts calculated", 15000, -30, 30);
  hTimesPerTrackVsPVal[m_InTrackColName] = new TH2D((std::string("hTimesPerTrackVsPVal") + m_InTrackColName).c_str(),
                                                    "per track time shifts calculated", 1500, -30, 30, 100, 0, 1);

  hDerivative[m_InTrackColName] = new TH1D((std::string("hDerivative") + m_InTrackColName).c_str(),
                                           "derivative both mirror hits;[cm/ns]", 1000, -0.02, 0.02);
  hWeightedDerivative[m_InTrackColName] = new TH1D(("hWeightedDerivative" + m_InTrackColName).c_str(), "derivative, weighted;[cm/ns]",
                                                   1000, -0.02, 0.02);
  hWeightedDerivativeBoth[m_InTrackColName] = new TH1D(("hWeightedDerivativeBoth" + m_InTrackColName).c_str(),
                                                       "derivative both mirror hits, weighted;[cm/ns]", 1000, -0.02, 0.02);
  hXTRelation[m_InTrackColName] = new TH2D(("hXTRelation" + m_InTrackColName).c_str(), ";measured drift distance;|dr/dt|", 500, -1, 1,
                                           1000, -0.008, 0.008);

  hEstimatedTimeErrors[m_InTrackColName] = new TH1D((std::string("hEstimatedTimeErrors") + m_InTrackColName).c_str(),
                                                    "errors of event time shifts calculated", 1500, 0, 30);

  hd2chi2da2[m_InTrackColName] = new TH1D((std::string("hd2chi2da2") + m_InTrackColName).c_str(),
                                          "d^{2}#chi^{2}/da^{2}", 1000, -.1, 20);

  TTree* T = t[m_InTrackColName] = new TTree((std::string("t") + m_InTrackColName).c_str(),
                                             "track data");
  // Need to add branches individually, to use variable size array.
  T->Branch("nTracks", &td.nTracks, "nTracks/I");
  T->Branch("pVal", td.pval, "pval[nTracks]/F");
  T->Branch("ndf", td.ndf, "ndf[nTracks]/F");
  T->Branch("px", td.px, "px[nTracks]/F");
  T->Branch("py", td.py, "py[nTracks]/F");
  T->Branch("pz", td.pz, "pz[nTracks]/F");
  T->Branch("pt", td.pt, "pt[nTracks]/F");
  T->Branch("parentPDG", td.parentPDG, "parentPDG[nTracks]/I");
  T->Branch("mcPDG", td.mcPDG, "mcPDG[nTracks]/I");
  T->Branch("PDG", td.PDG, "PDG[nTracks]/I");
  T->Branch("dchi2da", td.dchi2da, "dchi2da[nTracks]/D");
  T->Branch("d2chi2da2", td.d2chi2da2, "d2chi2da2[nTracks]/D");
}


void TrackTimeExtractionModule::beginRun()
{
}


void TrackTimeExtractionModule::event()
{
  StoreArray<genfit::Track> GFTracks(m_InTrackColName);
  StoreArray<genfit::TrackCand> GFTrackCands(m_InTrackCandColName);
  StoreArray<genfit::TrackCand> newTrackCands(m_OutTrackCandColName);
  StoreArray<MCParticle> mcParts("");
  RelationArray newTrackCandToOldTrack(newTrackCands, GFTracks);

  td.nTracks = 0;
  if (GFTracks.getEntries() > 100) {
    B2WARNING("More than 100 tracks in event, ignoring.");
  }

  double sumFirst = 0;
  double sumSecond = 0;
  double sumFirstCorrectHypot = 0;
  double sumSecondCorrectHypot = 0;
  double sumFirstGoodTracks = 0;
  double sumSecondGoodTracks = 0;
  double sumFirstFastTracks = 0;
  double sumSecondFastTracks = 0;
  for (int i = 0; i < GFTracks.getEntries(); ++i) {
    const genfit::Track* gfTrack = GFTracks[i];

    // Assume all tracks are eligible if no MC info.
    td.parentPDG[td.nTracks] = 0;
    td.mcPDG[td.nTracks] = 0;
    td.PDG[td.nTracks] = gfTrack->getCardinalRep()->getPDG();
    bool correctHypot = true;
    MCParticle* part = 0;
    if (mcParts.getEntries() > 0) {
      part = DataStore::getRelated<MCParticle>(gfTrack);
      if (part) {
        correctHypot = part && (fabs(part->getPDG()) == fabs(gfTrack->getCardinalRep()->getPDG()));
        td.mcPDG[td.nTracks] = part->getPDG();
        if (part->getMother())
          td.parentPDG[td.nTracks] = part->getMother()->getPDG();
      }
    }

    for (size_t i = 0; i < gfTrack->getNumPointsWithMeasurement(); ++i) {
      if (!dynamic_cast<const CDCRecoHit*>(gfTrack->getPointWithMeasurement(i))) {
        B2DEBUG(200, "No CDC hits in track.");
        continue;
      }
    }

    bool goodTrack = false;
    bool fastTrack = false;
    double PVal = 0;
    bool fitSuccess = gfTrack->hasFitStatus();
    if (fitSuccess) {
      genfit::FitStatus* fs = 0;
      genfit::KalmanFitStatus* kfs = 0;
      fs = gfTrack->getFitStatus();
      if (!fs)
        continue;
      if (fs->isTrackPruned()) {
        B2WARNING("Skipping pruned track");
        continue;
      }
      fitSuccess = fitSuccess && fs->isFitted();
      fitSuccess = fitSuccess && fs->isFitConverged();
      kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
      fitSuccess = fitSuccess && kfs;
      if (!fitSuccess)
        continue;
      td.pval[td.nTracks] = fs->getPVal();
      td.ndf[td.nTracks] = fs->getNdf();
      goodTrack = td.pval[td.nTracks] > 0.01;
      TVector3 mom;
      try {
        mom = gfTrack->getFittedState().getMom();
      } catch (...) {
        continue;
      }
      fastTrack = mom.Mag() > 0.5;
      td.px[td.nTracks] = mom.X();
      td.py[td.nTracks] = mom.Y();
      td.pz[td.nTracks] = mom.Z();
      td.pt[td.nTracks] = mom.Perp();
    }

    try {
      std::vector<int> vDimMeas;
      getMeasurementDimensions(gfTrack, vDimMeas);

      TMatrixDSym fullCovariance;
      bool success = buildFullCovarianceMatrix(gfTrack, fullCovariance);
      if (!success) {
        // Error printed inside.
        continue;
      }
      TMatrixDSym fullResidualCovariance;
      TMatrixDSym inverseFullMeasurementCovariance;
      buildFullResidualCovarianceMatrix(gfTrack, vDimMeas, fullCovariance, fullResidualCovariance,
                                        inverseFullMeasurementCovariance);

      TVectorD residuals;
      TVectorD residualsTimeDerivative;
      buildResidualsAndTimeDerivative(gfTrack, vDimMeas, residuals, residualsTimeDerivative);

      // Equations with their numbers from 0810.2241:
      // 2 At V^-1 (V - HCHt) V^-1 r       (9)
      // = 2 At V^-1 r for fitted tracks   (12)
      double dchi2da = 2. * residualsTimeDerivative * (inverseFullMeasurementCovariance * residuals);
      td.dchi2da[td.nTracks] = dchi2da;

      // (2 At V^-1 (V - HCHt) V^-1 A)^-1, note that this should be   (10)
      // SimilarityT(...) if (...) were a matrix.
      double d2chi2da2 = 2. * fullResidualCovariance.Similarity(inverseFullMeasurementCovariance * residualsTimeDerivative);
      td.d2chi2da2[td.nTracks] = d2chi2da2;

      if (m_DoHistogramming)
        hd2chi2da2[m_InTrackColName]->Fill(d2chi2da2);

      if (m_DoHistogramming) {
        hTimesPerTrack[m_InTrackColName]->Fill(1 / d2chi2da2 * dchi2da);
        hTimesPerTrackVsPVal[m_InTrackColName]->Fill(1 / d2chi2da2 * dchi2da, PVal);
      }
      if (d2chi2da2 > 20) {
        B2DEBUG(200, "Track with bad second derivative");
        continue;
      }
      sumFirst += d2chi2da2;
      sumSecond += dchi2da;
      if (correctHypot) {
        sumFirstCorrectHypot += d2chi2da2;
        sumSecondCorrectHypot += dchi2da;
      }
      if (goodTrack) {
        sumFirstGoodTracks += d2chi2da2;
        sumSecondGoodTracks += dchi2da;
      }
      // High momentum and reasonable second derivative
      // --> these are the tracks that we want to use for the event time
      //     extraction.
      if (fastTrack && d2chi2da2 < 20) {
        sumFirstFastTracks += d2chi2da2;
        sumSecondFastTracks += dchi2da;
      }
    } catch (...) {
      B2ERROR("Failed track");
    }
    td.nTracks++;
  }
  if (m_DoHistogramming) {
    hEstimatedTimeErrors[m_InTrackColName]->Fill(sqrt(2 / sumFirst));
    hTimes[m_InTrackColName]->Fill(1 / sumFirst * sumSecond);
    hTimesCorrectHypot[m_InTrackColName]->Fill(1 / sumFirstCorrectHypot * sumSecondCorrectHypot);
    hTimesGoodTracks[m_InTrackColName]->Fill(1 / sumFirstGoodTracks * sumSecondGoodTracks);
    hTimesFastTracks[m_InTrackColName]->Fill(1 / sumFirstFastTracks * sumSecondFastTracks);

    if (td.nTracks > 0)
      t[m_InTrackColName]->Fill();
  }
  for (int i = 0; i < GFTracks.getEntries(); ++i) {
    const genfit::TrackCand* oldCand = DataStore::getRelated<genfit::TrackCand>(GFTracks[i], m_InTrackCandColName);
    auto cand = newTrackCands.appendNew(*oldCand);
    cand->setTimeSeed(cand->getTimeSeed() + 1 / sumFirst * sumSecond);
    newTrackCandToOldTrack.add(i, i);
  }
}


void TrackTimeExtractionModule::endRun()
{
}


void TrackTimeExtractionModule::terminate()
{
}


void
TrackTimeExtractionModule::getMeasurementDimensions(const genfit::Track* tr,
                                                    std::vector<int>& vDimMeas)
{
  vDimMeas.resize(tr->getNumPointsWithMeasurement());
  for (size_t i = 0; i < tr->getNumPointsWithMeasurement(); ++i) {
    vDimMeas.push_back(tr->getPointWithMeasurement(i)->getRawMeasurement(0)->getDim());
  }
}


namespace {
  /** TMatrixTSym::SetSub is sloooooowwwww as it uses
      TMatrixT::operator() which does bounds checking.  Hence this
      homegrown alternative.  */
  template <typename T>
  void setSubOnDiagonal(TMatrixTSym<T>& target, int iRow,
                        const TMatrixTSym<T>& source)
  {
    const int nColsTarget = target.GetNrows();
    const int nColsSource = source.GetNrows();

    T* pt = target.GetMatrixArray();
    const T* ps = source.GetMatrixArray();

    for (int i = 0; i < nColsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iRow + i)*nColsTarget + (iRow + j)] = ps[i * nColsSource + j];
      }
    }
  }

  /** Insert a TMatrixT somewhere off-diagonal into a TMatrixTSym.
      This takes care of the mirrored part.
      So the source matrix is inserted starting from the (iRow, iCol) element
      and the transposed               starting from the (iCol, iRow) element  */
  template <typename T>
  void setSubOffDiagonal(TMatrixTSym<T>& target, int iRow, int iCol,
                         const TMatrixT<T>& source)
  {
    const int nColsTarget = target.GetNcols();
    const int nRowsSource = source.GetNrows();
    const int nColsSource = source.GetNcols();

    T* pt = target.GetMatrixArray();
    const T* ps = source.GetMatrixArray();

    for (int i = 0; i < nRowsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iRow + i)*nColsTarget + (iCol + j)] = ps[i * nColsSource + j];
      }
    }
    for (int i = 0; i < nRowsSource; ++i) {
      for (int j = 0; j < nColsSource; ++j) {
        pt[(iCol + j)*nColsTarget + (iRow + i)] = ps[i * nColsSource + j];
      }
    }
  }
}

bool TrackTimeExtractionModule::buildFullCovarianceMatrix(const genfit::Track* tr,
                                                          TMatrixDSym& fullCovariance)
{
  if (!tr->hasKalmanFitStatus()) {
    B2ERROR("Track not fitted with a Kalman fitter.");
    return false;
  }

  const genfit::KalmanFitStatus* kfs = tr->getKalmanFitStatus();
  if (!kfs->isFitConverged()) {
    B2ERROR("Track fit didn't converge.");
    return false;
  }

  if (!kfs->isFittedWithReferenceTrack()) {
    B2ERROR("No reference track.");
    return false;
  }

  const unsigned int nPoints = tr->getNumPoints();
  const genfit::AbsTrackRep* rep = tr->getCardinalRep();
  const int nDim = rep->getDim();

  fullCovariance.ResizeTo(nPoints * nDim, nPoints * nDim);
  std::vector<TMatrixD> vFitterGain;
  vFitterGain.reserve(nPoints);
  for (unsigned int i = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = tr->getPoint(i);
    const genfit::KalmanFitterInfo* fi = tp->getKalmanFitterInfo();
    if (!fi) {
      B2ERROR("Missing KalmanFitterInfo");
      return false;
    }

    // Diagonal part of the full covariance matrix are the covariances
    // of the smoothed states.
    const genfit::MeasuredStateOnPlane& mop = fi->getFittedState();
    setSubOnDiagonal(fullCovariance, i * nDim, mop.getCov());

    // Build the corresponding smoother gain matrix.
    if (i + 1 < nPoints) {
      const genfit::TrackPoint* tpNext = tr->getPoint(i + 1);
      const genfit::KalmanFitterInfo* fiNext = tpNext->getKalmanFitterInfo();
      if (!fiNext) {
        B2ERROR("Missing next KalmanFitterInfo");
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

bool
TrackTimeExtractionModule::buildFullResidualCovarianceMatrix(const genfit::Track* tr,
    const std::vector<int>& vDimMeas,
    const TMatrixDSym& fullCovariance,
    TMatrixDSym& fullResidualCovariance,
    TMatrixDSym& inverseFullMeasurementCovariance)
{
  /* The fullResidualCovariance is eq. (17) in 0810.2241.  */

  const unsigned int nPoints = tr->getNumPoints();
  const genfit::AbsTrackRep* rep = tr->getCardinalRep();
  const int nDim = rep->getDim();
  int measurementDimensions = std::accumulate(vDimMeas.begin(), vDimMeas.end(), 0);
  fullResidualCovariance.ResizeTo(measurementDimensions, measurementDimensions);
  inverseFullMeasurementCovariance.ResizeTo(measurementDimensions, measurementDimensions);

  // Put together the parts containing the track covariances.
  std::vector<TMatrixD> HMatrices;
  HMatrices.reserve(nPoints);
  for (unsigned int i = 0, index = 0; i < nPoints; ++i) {
    const genfit::TrackPoint* tp = tr->getPoint(i);
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
    const genfit::TrackPoint* tp = tr->getPoint(i);
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


void
TrackTimeExtractionModule::buildResidualsAndTimeDerivative(const genfit::Track* tr,
                                                           const std::vector<int>& vDimMeas,
                                                           TVectorD& residuals,
                                                           TVectorD& residualTimeDerivative)
{
  // The residuals and their derivatives WRT the event time.

  int measurementDimensions = std::accumulate(vDimMeas.begin(), vDimMeas.end(), 0);
  residuals.ResizeTo(measurementDimensions);
  residualTimeDerivative.ResizeTo(measurementDimensions);

  for (unsigned int i = 0, index = 0; i < tr->getNumPoints(); ++i) {
    const genfit::TrackPoint* tp = tr->getPoint(i);
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

      if (m_DoHistogramming) {
        hDerivative[m_InTrackColName]->Fill(deriv[0]);
        hDerivative[m_InTrackColName]->Fill(deriv[1]);
        hWeightedDerivativeBoth[m_InTrackColName]->Fill(deriv[0], weights[0]);
        hWeightedDerivativeBoth[m_InTrackColName]->Fill(deriv[1], weights[1]);
        hWeightedDerivative[m_InTrackColName]->Fill(weightedDeriv);

        auto vmops = hit->constructMeasurementsOnPlane(fi->getFittedState());
        hXTRelation[m_InTrackColName]->Fill(vmops[0]->getState()(0), deriv[0]);
        hXTRelation[m_InTrackColName]->Fill(vmops[1]->getState()(0), deriv[1]);
        delete vmops[0];
        delete vmops[1];
      }
    }
    index += vDimMeas[i];
  }
}
