/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "testbeam/vxd/modules/DQM/TrackfitDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <vxd/geometry/SensorPlane.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/HMatrixU.h>
#include <genfit/HMatrixV.h>

#include <set>
#include <boost/format.hpp>

#include "TDirectory.h"

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackfitDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackfitDQMModule::TrackfitDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("Track fit DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("GFTrackCandidatesColName", m_storeTrackCandName,
           "Name of collection holding the genfit::TrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", std::string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit Validation histograms will be put into this directory", std::string("trackfit"));
  addParam("fillExpertHistograms", m_fillExpertHistos,
           "Fill additional histograms", false);
}


TrackfitDQMModule::~TrackfitDQMModule()
{
  m_allHistos.clear();
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackfitDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  TDirectory* histDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
  histDir->cd();

  m_hNTracks = getHist("hNTracks", "number of tracks", 3, 0, 3);
  m_hNTracks->GetXaxis()->SetBinLabel(1, "fitted, converged");
  m_hNTracks->GetXaxis()->SetBinLabel(2, "fitted, not converged");
  m_hNTracks->GetXaxis()->SetBinLabel(3, "TrackCand, but no Track");
  m_hNDF = getHist("hNDF", "NDF of fitted track;NDF;Tracks", 15, -5, 10);
  m_hChi2 = getHist("hChi2", "#chi^{2} of tracks;#chi^{2};Tracks", 1000, 0, 20);
  m_hPval = getHist("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);
  if (m_fillExpertHistos) {
    m_hNDFChi2 = getHist("hNDFChi2", "#chi^{2} of tracks;NDF;#chi^{2};Tracks", 8, 0, 8, 1000, 0, 20);
    m_hNDFPval = getHist("hNDFPval", "p-values of tracks;NDF;pVal;Tracks", 8, 0, 8, 1000, 0, 1);
  }
  for (int i = 0; i < c_nPlanes; ++i) {
    int iLayer = indexToPlane(i);
    std::string title, name;

    const double residualRange = 0.1;

    name = (boost::format("hResidualsU%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along U in layer %1%;cm;Tracks") % iLayer).str();
    m_hResidualU[i] = getHist(name, title, 1000, -residualRange, residualRange);
    name = (boost::format("hResidualsV%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along V in layer %1%;cm;Tracks") % iLayer).str();
    m_hResidualV[i] = getHist(name, title, 1000, -residualRange, residualRange);
    name = (boost::format("hResidualVvsU%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along V vs U in layer %1%;res V [cm];U [cm];Tracks") % iLayer).str();
    m_hResidualVvsU[i] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = (boost::format("hResidualUvsV%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along U vs V in layer %1%;res U [cm];V [cm];Tracks") % iLayer).str();
    m_hResidualUvsV[i] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = (boost::format("hResidualUvsU%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along U vs U in layer %1%;res U [cm];U [cm];Tracks") % iLayer).str();
    m_hResidualUvsU[i] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = (boost::format("hResidualVvsV%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along V vs V in layer %1%;res V [cm];V [cm];Tracks") % iLayer).str();
    m_hResidualVvsV[i] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = (boost::format("hNormalizedResidualsU%1%") % iLayer).str();
    title = (boost::format("normalized, unbiased residuals in layer %1%;#sigma;Tracks") % iLayer).str();

    double normResRange = 5;
    m_hNormalizedResidualU[i] = getHist(name, title, 1000, -normResRange, normResRange);
    name = (boost::format("hNormalizedResidualsV%1%") % iLayer).str();
    title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma;Tracks") % iLayer).str();
    m_hNormalizedResidualV[i] = getHist(name, title, 1000, -normResRange, normResRange);
    if (m_fillExpertHistos) {
      name = (boost::format("hNDFResidualsU%1%") % iLayer).str();
      title = (boost::format("unnormalized, unbiased residuals along U in layer %1%;NDF;cm;Tracks") % iLayer).str();
      m_hNDFResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);
      name = (boost::format("hNDFResidualsV%1%") % iLayer).str();
      title = (boost::format("unnormalized, unbiased residuals along V in layer %1%;NDF;cm;Tracks") % iLayer).str();
      m_hNDFResidualV[i] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);
      name = (boost::format("hNDFNormalizedResidualsU%1%") % iLayer).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma;Tracks") % iLayer).str();
      m_hNDFNormalizedResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
      name = (boost::format("hNDFNormalizedResidualsV%1%") % iLayer).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma;Tracks") % iLayer).str();
      m_hNDFNormalizedResidualV[i] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
    }
  }

  m_hPseudoEfficienciesU = getHistProfile("hPseudoEfficienciesU", "pseudo efficiencies U;layer;pseudo efficiency", 6, 1, 7);
  m_hPseudoEfficienciesV = getHistProfile("hPseudoEfficienciesV", "pseudo efficiencies V;layer;pseudo efficiency", 6, 1, 7);

  for (int i = 0; i < 6; ++i) {
    const char* varName[6] = { "x", "y", "z", "px", "py", "pz", };
    const char* varTitle[6] = { "x", "y", "z", "p_{x}", "p_{y}", "p_{z}", };
    double lower[6] = { -1., -3., -3, -6, -1., -1., };
    double upper[6] = {  1.,  3.,  3,  6,  1.,  1., };

    std::string name, title;
    name = (boost::format("hSeedQuality%1%") % varName[i]).str();
    title = (boost::format("seed vs. trackfit result for %1%;seed;reco track;events") % varTitle[i]).str();
    m_hSeedQuality[i] = getHist(name, title, 100, lower[i], upper[i], 100, lower[i], upper[i]);
  }
  oldDir->cd();
}


void TrackfitDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<genfit::Track> storeTrack(m_storeTrackName);
  StoreArray<genfit::TrackCand> storeTrackCand(m_storeTrackCandName);

  //storeClusters.required();
  //storeDigits.required();

  RelationArray relTrackCandTrack(storeTrackCand, storeTrack);

  //Store names to speed up creation later
  m_storeTrackName = storeTrack.getName();
  m_storeTrackCandName = storeTrackCand.getName();
  m_relTrackCandTrackName = relTrackCandTrack.getName();
}

void TrackfitDQMModule::beginRun()
{
  for (size_t i = 0; i < m_allHistos.size(); ++i) {
    m_allHistos[i]->Reset();
  }

  findPlanes();
}


void TrackfitDQMModule::event()
{
  const StoreArray<genfit::Track> storeTrack(m_storeTrackName);
  const StoreArray<genfit::TrackCand> storeTrackCand(m_storeTrackCandName);

  const RelationArray relTrackCandTrack(storeTrackCand, storeTrack, m_relTrackCandTrackName);

  // Loop over TrackCands
  for (int i = 0; i < storeTrackCand.getEntries(); ++i) {
    const genfit::TrackCand* cand = storeTrackCand[i];
    const genfit::Track* track = DataStore::getRelatedFromObj<genfit::Track>(cand);
    if (!track) {
      // No related Track
      m_hNTracks->Fill("TrackCand, but no Track", 1.0);
      continue;
    }
    if (!track->checkConsistency())
      return;

    const genfit::FitStatus* fs = track->getFitStatus();
    if (!fs || !fs->isFitConverged()) {
      m_hNTracks->Fill("fitted, not converged", 1.0);
      continue;
    }
    m_hNTracks->Fill("fitted, converged", 1.0);
    double NDF = fs->getNdf();
    m_hNDF->Fill(NDF);
    m_hChi2->Fill(fs->getChi2());
    m_hPval->Fill(fs->getPVal());
    if (m_fillExpertHistos) {
      m_hNDFChi2->Fill(NDF, fs->getChi2());
      m_hNDFPval->Fill(NDF, fs->getPVal());
    }

    // Unbiased residuals don't make much sense with less d-o-f than
    // the dimension of the hit being removed.
    if (NDF < 1)
      continue;

    plotResiduals(track);
    plotPseudoEfficiencies(track);
    plotSeedQuality(track);
  }
}


void TrackfitDQMModule::endRun()
{
}


void TrackfitDQMModule::terminate()
{
}

void TrackfitDQMModule::findPlanes()
{
  m_vPlanes.clear();

  for (int i = 0; i < c_nPlanes; ++i) {
    ROIDetPlane plane(getInfo(i).getID());
    const VXD::SensorPlane& finitePlane(getInfo(i).getID());
    plane.setFinitePlane(finitePlane.clone());
    m_vPlanes.push_back(plane);
  }

  B2DEBUG(2, "found " << m_vPlanes.size() << " layers.");
}


void TrackfitDQMModule::plotResiduals(const genfit::Track* track)
{
  double NDF = track->getKalmanFitStatus()->getNdf();
  for (unsigned int i = 0; i < track->getNumPoints(); ++i) {
    const genfit::TrackPoint* tp = track->getPoint(i);
    if (!tp->hasRawMeasurements())
      continue;

    // Identify Layer.
    const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
    const SVDRecoHit* rawSVD = dynamic_cast<const SVDRecoHit*>(raw);
    const PXDRecoHit* rawPXD = dynamic_cast<const PXDRecoHit*>(raw);

    if (rawSVD) {
      int layer = rawSVD->getSensorID().getLayerNumber();
      int index = planeToIndex(layer);

      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi)
        continue;

      double u = 0;
      double v = 0;
      double resU = 0;
      double resV = 0;
      bool haveU = false;
      bool haveV = false;

      unsigned int nMeasurements = kfi->getNumMeasurements();
      for (unsigned int iMeas = 0; iMeas < nMeasurements; ++iMeas) {
        const genfit::MeasurementOnPlane& res = kfi->getResidual(iMeas, false);
        const genfit::AbsHMatrix* h = kfi->getMeasurementOnPlane(iMeas)->getHMatrix();
        bool isU = dynamic_cast<const genfit::HMatrixU*>(h);
        bool isV = dynamic_cast<const genfit::HMatrixV*>(h);
        if (isU) {
          if (NDF > 2) {
            haveU = haveU || isU;
            u = kfi->getFittedState().getState()(3);
            resU = res.getState()(0);
            m_hResidualU[index]->Fill(resU,
                                      res.getWeight());
            m_hNormalizedResidualU[index]->Fill(resU / sqrt(res.getCov()(0, 0)),
                                                res.getWeight());
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualU[index]->Fill(NDF,
                                         res.getState()(0),
                                         res.getWeight());
            m_hNDFNormalizedResidualU[index]->Fill(NDF,
                                                   res.getState()(0) / sqrt(res.getCov()(0, 0)),
                                                   res.getWeight());
          }
        } else if (isV) {
          if (NDF > 2) {
            haveV = haveV || isV;
            v = kfi->getFittedState().getState()(4);
            resV = res.getState()(0);
            m_hResidualV[index]->Fill(resV,
                                      res.getWeight());
            m_hNormalizedResidualV[index]->Fill(resV / sqrt(res.getCov()(0, 0)),
                                                res.getWeight());
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualV[index]->Fill(NDF,
                                         res.getState()(0),
                                         res.getWeight());
            m_hNDFNormalizedResidualV[index]->Fill(NDF,
                                                   res.getState()(0) / sqrt(res.getCov()(0, 0)),
                                                   res.getWeight());
          }
        }
      }
      if (NDF > 2) {
        if (haveU && haveV) {
          m_hResidualUvsV[index]->Fill(resU, v);
          m_hResidualVvsU[index]->Fill(resV, u);
        }
        if (haveU)
          m_hResidualUvsU[index]->Fill(resU, u);
        if (haveV)
          m_hResidualVvsV[index]->Fill(resV, v);
      }
    }

    if (rawPXD) {
      if (rawSVD) {
        // Cannot happen.
        B2WARNING("Hit is both SVD and PXD hit");
        continue;
      }

      int layer = rawPXD->getSensorID().getLayerNumber();
      int index = planeToIndex(layer);

      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi)
        continue;

      unsigned int nMeasurements = kfi->getNumMeasurements();
      if (nMeasurements != 1)
        continue;

      for (unsigned int iMeas = 0; iMeas < nMeasurements; ++iMeas) {
        const genfit::MeasurementOnPlane& res = kfi->getResidual(iMeas, false);
        const genfit::AbsHMatrix* h = kfi->getMeasurementOnPlane(iMeas)->getHMatrix();
        if (!dynamic_cast<const genfit::HMatrixUV*>(h))
          continue;

        double u = kfi->getFittedState().getState()(3);
        double v = kfi->getFittedState().getState()(3);
        double resU = res.getState()(0);
        double errU = sqrt(res.getCov()(0, 0));
        double resV = res.getState()(1);
        double errV = sqrt(res.getCov()(1, 1));

        if (NDF > 3) {
          m_hResidualU[index]->Fill(resU, res.getWeight());
          m_hNormalizedResidualU[index]->Fill(resU / errU, res.getWeight());

          m_hResidualV[index]->Fill(resV, res.getWeight());
          m_hNormalizedResidualV[index]->Fill(resV / errV, res.getWeight());

          m_hResidualUvsV[index]->Fill(resU, v);
          m_hResidualVvsU[index]->Fill(resV, u);
          m_hResidualUvsU[index]->Fill(resU, u);
          m_hResidualVvsV[index]->Fill(resV, v);

        }

        if (m_fillExpertHistos) {
          m_hNDFResidualU[index]->Fill(NDF, resU, res.getWeight());
          m_hNDFNormalizedResidualU[index]->Fill(NDF, resU / errU, res.getWeight());

          m_hNDFResidualV[index]->Fill(NDF, resV, res.getWeight());
          m_hNDFNormalizedResidualV[index]->Fill(NDF, resV / errV, res.getWeight());
        }
      }
    }
  }
}

void TrackfitDQMModule::plotPseudoEfficiencies(const genfit::Track* track)
{
  double PVal = track->getKalmanFitStatus()->getPVal();
  if (PVal < .1)
    return;

  // Extrapolate to all planes, see if there should be a hit.
  genfit::MeasuredStateOnPlane mop;
  try { mop = track->getFittedState(); } catch (...) { return; }
  for (int i = 0; i < c_nPlanes; ++i) {
    //const SVD::SensorInfo& info = getInfo(i);
    //const VXD::SensorPlane& plane(info.getID());

    try {
      mop.extrapolateToPlane(genfit::SharedPlanePtr(new ROIDetPlane(m_vPlanes[i])));
      if (m_vPlanes[i].isInActive(mop.getPos(), mop.getMom())) {

        m_hPseudoEfficienciesU->Fill(indexToPlane(i), 0);
        m_hPseudoEfficienciesV->Fill(indexToPlane(i), 0);
      }
    } catch (...) {}
  }

  // Go through all hits, record planes with hits
  for (unsigned int i = 0; i < track->getNumPointsWithMeasurement(); ++i) {
    const genfit::TrackPoint* tp = track->getPointWithMeasurement(i);

    // Identify Layer.
    const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
    const SVDRecoHit* rawSVD = dynamic_cast<const SVDRecoHit*>(raw);
    const PXDRecoHit* rawPXD = dynamic_cast<const PXDRecoHit*>(raw);
    if (!rawSVD && !rawPXD)
      continue;

    const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
    if (!kfi)
      continue;

    unsigned int nMeasurements = kfi->getNumMeasurements();
    for (unsigned int iMeas = 0; iMeas < nMeasurements; ++iMeas) {
      const genfit::AbsHMatrix* h = kfi->getMeasurementOnPlane(iMeas)->getHMatrix();
      if (dynamic_cast<const genfit::HMatrixU*>(h)) {
        m_hPseudoEfficienciesU->Fill(indexToPlane(i), 2);
      } else if (dynamic_cast<const genfit::HMatrixV*>(h)) {
        m_hPseudoEfficienciesV->Fill(indexToPlane(i), 2);
      } else if (dynamic_cast<const genfit::HMatrixUV*>(h)) {
        m_hPseudoEfficienciesU->Fill(indexToPlane(i), 2);
        m_hPseudoEfficienciesV->Fill(indexToPlane(i), 2);
      }
    }

  }
}

void TrackfitDQMModule::plotSeedQuality(const genfit::Track* track)
{
  genfit::MeasuredStateOnPlane mop;
  try { mop = track->getFittedState(); } catch (...) { return; }
  const TVectorD& seedState = track->getStateSeed();
  TVector3 seedPos(seedState(0), seedState(1), seedState(2));
  TVector3 seedMom(seedState(3), seedState(4), seedState(5));
  try {
    mop.extrapolateToPoint(seedPos);
    TVector3 pos;
    TVector3 mom;
    TMatrixDSym cov(6);
    mop.getPosMomCov(pos, mom, cov);
    for (int i = 0; i < 3; ++i) {
      m_hSeedQuality[i]->Fill(seedPos(i), pos(i));
      m_hSeedQuality[i + 3]->Fill(seedMom(i), mom(i));
    }
  } catch (...) { return; }
}


const VXD::SensorInfoBase& TrackfitDQMModule::getInfo(int index) const
{
  int iPlane = indexToPlane(index);
  VxdID sensorID(iPlane, 1, iPlane);
  return dynamic_cast<const VXD::SensorInfoBase&>(VXD::GeoCache::get(sensorID));
}
