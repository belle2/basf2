/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *    - several changes to make it work with the 2016 test beam           *
 *      (Thomas Lueck,  committed March 2016)                             *
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
#include "TString.h"

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
           "Name of collection holding the genfit::TrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)",
           std::string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit Validation histograms will be put into this directory", std::string("trackfit"));
  addParam("fillExpertHistograms", m_fillExpertHistos,
           "Fill additional histograms", false);
}


TrackfitDQMModule::~TrackfitDQMModule()
{
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


  for (VxdID& avxdid : m_sensorList) {
    TString title, name;

    const double residualRange = 0.1;
    name = "hResidualsU" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along U in Sesnor " + ((std::string)avxdid);
    m_hResidualU[avxdid] = getHist(name, title, 1000, -residualRange, residualRange);
    name = "hResidualsV" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along V in Sensor " + ((std::string)avxdid) + " ;cm;Tracks";
    m_hResidualV[avxdid] = getHist(name, title, 1000, -residualRange, residualRange);
    name = "hResidualVvsU" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along V vs U in Sensori " + ((std::string)avxdid) + " ;res V [cm];U [cm];Tracks";
    m_hResidualVvsU[avxdid] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = "hResidualUvsV" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along U vs V in Sensor " + ((std::string)avxdid) + ";res U [cm];V [cm];Tracks";
    m_hResidualUvsV[avxdid] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = "hResidualUvsU" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along U vs U in Sensor " + ((std::string)avxdid) + ";res U [cm];U [cm];Tracks";
    m_hResidualUvsU[avxdid] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = "hResidualVvsV" + ((std::string)avxdid);
    title = "unnormalized, unbiased residuals along V vs V in Sensor " + ((std::string)avxdid) + ";res V [cm];V [cm];Tracks";
    m_hResidualVvsV[avxdid] = getHist(name, title, 100, -residualRange, residualRange, 100, -5, 5);
    name = "hNormalizedResidualsU" + ((std::string)avxdid);
    title = "normalized, unbiased residuals in Sensor " + ((std::string)avxdid) + ";#sigma;Tracks";

    double normResRange = 5;
    m_hNormalizedResidualU[avxdid] = getHist(name, title, 1000, -normResRange, normResRange);
    name = "hNormalizedResidualsV" + ((std::string)avxdid);
    title = "normalized, unbiased residuals in Sensor " + ((std::string)avxdid) + ";NDF;#sigma;Tracks";
    m_hNormalizedResidualV[avxdid] = getHist(name, title, 1000, -normResRange, normResRange);
    if (m_fillExpertHistos) {
      name = "hNDFResidualsU" + ((std::string)avxdid);
      title = "unnormalized, unbiased residuals along U in Sensor " + ((std::string)avxdid) + ";NDF;cm;Tracks";
      m_hNDFResidualU[avxdid] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);
      name = "hNDFResidualsV" + ((std::string)avxdid);
      title = "unnormalized, unbiased residuals along V in Sensor " + ((std::string)avxdid) + ";NDF;cm;Tracks";
      m_hNDFResidualV[avxdid] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);
      name = "hNDFNormalizedResidualsU" + ((std::string)avxdid);
      title = "normalized, unbiased residuals in Sensor " + ((std::string)avxdid) + ";NDF;#sigma;Tracks";
      m_hNDFNormalizedResidualU[avxdid] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
      name = "hNDFNormalizedResidualsV" + ((std::string)avxdid);
      title = "normalized, unbiased residuals in Sensor " + ((std::string)avxdid) + ";NDF;#sigma;Tracks";
      m_hNDFNormalizedResidualV[avxdid] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
    }
  }

  m_hPseudoEfficienciesU = getHistProfile("hPseudoEfficienciesU", "pseudo efficiencies U;layer;pseudo efficiency", 6, 1, 7);
  m_hPseudoEfficienciesV = getHistProfile("hPseudoEfficienciesV", "pseudo efficiencies V;layer;pseudo efficiency", 6, 1, 7);

  m_hMomentum = getHist("hMomentum", "reconstructed momentum at first TrackPoint", 1000, 0, 8);

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
  //get the list of sensors here or earlier as other methods need a it filled
  m_sensorList = VXD::GeoCache::getInstance().getListOfSensors();


  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<genfit::Track> storeTrack(m_storeTrackName);
  StoreArray<genfit::TrackCand> storeTrackCand(m_storeTrackCandName);

  //storeClusters.isRequired();
  //storeDigits.isRequired();

  RelationArray relTrackCandTrack(storeTrackCand, storeTrack);

  //Store names to speed up creation later
  m_storeTrackName = storeTrack.getName();
  m_storeTrackCandName = storeTrackCand.getName();
  m_relTrackCandTrackName = relTrackCandTrack.getName();
}

void TrackfitDQMModule::beginRun()
{
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
    const genfit::Track* track = DataStore::getRelated<genfit::Track>(cand, m_storeTrackName);
    if (!track) {
      // No related Track
      m_hNTracks->Fill("TrackCand, but no Track", 1.0);
      continue;
    }

    try {
      track->checkConsistency();
    } catch (genfit::Exception& e) {
      return;
    }

    /* //Data integrity check:
    const genfit::TrackCand* candBackwards = DataStore::getRelatedToObj<genfit::TrackCand>(track, m_storeTrackCandName);
    if (!candBackwards)
      B2ERROR("backwards lookup failed");
    */

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

    bool haveFittedState = true;
    genfit::MeasuredStateOnPlane mop = 0;
    try { mop = track->getFittedState(); } catch (...) { haveFittedState = false; }
    if (haveFittedState) {
      m_hMomentum->Fill(mop.getMomMag());
    }

    // Unbiased residuals don't make much sense with less d-o-f than
    // the dimension of the hit being removed.
    if (NDF < 1)
      continue;

    try {
      plotResiduals(track);
      plotPseudoEfficiencies(track);
      plotSeedQuality(track);
    } catch (...) {
      // Skip.
    }

  }
}


void TrackfitDQMModule::endRun()
{
}


void TrackfitDQMModule::terminate()
{
  //just a bit cosmetics
  for (int i = 0; i < (int)m_allHistos.size(); i++) {
    if (m_allHistos[i]->GetEntries() <= 0) {
      B2DEBUG(1, "Deleting histogram " << m_allHistos[i]->GetName());
      if (m_allHistos[i]) delete  m_allHistos[i];
    }
  }
}

void TrackfitDQMModule::findPlanes()
{
  m_vPlanes.clear();

  for (int i = 0; i < (int)m_sensorList.size(); ++i) {
    ROIDetPlane plane(m_sensorList[i]);
    const VXD::SensorPlane& finitePlane(m_sensorList[i]);
    plane.setFinitePlane(finitePlane.clone());
    m_vPlanes[m_sensorList[i]] = plane;
  }

  B2DEBUG(2, "found " << m_vPlanes.size() << " layers.");
}




void TrackfitDQMModule::plotResiduals(const genfit::Track* track)
{
  double NDF = track->getKalmanFitStatus()->getNdf();
  for (unsigned int i = 0; i < track->getNumPoints(); ++i) {
    const genfit::TrackPoint* tp = track->getPoint(i);

    if (!tp->hasRawMeasurements()) {
      B2DEBUG(1, "no raw measurements");
      continue;
    }

    // Identify Layer.
    const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
    const SVDRecoHit* rawSVD = dynamic_cast<const SVDRecoHit*>(raw);
    const PXDRecoHit* rawPXD = dynamic_cast<const PXDRecoHit*>(raw);

    if (rawSVD) {
      VxdID avxdid = rawSVD->getSensorID();

      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi) {
        B2DEBUG(1, "No kalmanfitterinfo");
        continue;
      }

      double u = 0;
      double v = 0;
      double resU = 0;
      double resV = 0;
      bool haveU = false;
      bool haveV = false;

      unsigned int nMeasurements = kfi->getNumMeasurements();
      B2DEBUG(1, "n Measurements " << nMeasurements);
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
            m_hResidualU[avxdid]->Fill(resU,
                                       res.getWeight());
            m_hNormalizedResidualU[avxdid]->Fill(resU / sqrt(res.getCov()(0, 0)),
                                                 res.getWeight());
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualU[avxdid]->Fill(NDF,
                                          res.getState()(0),
                                          res.getWeight());
            m_hNDFNormalizedResidualU[avxdid]->Fill(NDF,
                                                    res.getState()(0) / sqrt(res.getCov()(0, 0)),
                                                    res.getWeight());
          }
        } else if (isV) {
          if (NDF > 2) {
            haveV = haveV || isV;
            v = kfi->getFittedState().getState()(4);
            resV = res.getState()(0);
            m_hResidualV[avxdid]->Fill(resV,
                                       res.getWeight());
            m_hNormalizedResidualV[avxdid]->Fill(resV / sqrt(res.getCov()(0, 0)),
                                                 res.getWeight());
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualV[avxdid]->Fill(NDF,
                                          res.getState()(0),
                                          res.getWeight());
            m_hNDFNormalizedResidualV[avxdid]->Fill(NDF,
                                                    res.getState()(0) / sqrt(res.getCov()(0, 0)),
                                                    res.getWeight());
          }
        }
      }
      if (NDF > 2) {
        if (haveU && haveV) {
          m_hResidualUvsV[avxdid]->Fill(resU, v);
          m_hResidualVvsU[avxdid]->Fill(resV, u);
        }
        if (haveU)
          m_hResidualUvsU[avxdid]->Fill(resU, u);
        if (haveV)
          m_hResidualVvsV[avxdid]->Fill(resV, v);
      }
    }

    if (rawPXD) {
      if (rawSVD) {
        // Cannot happen.
        B2WARNING("Hit is both SVD and PXD hit");
        continue;
      }

      VxdID avxdid = rawPXD->getSensorID();

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
        double v = kfi->getFittedState().getState()(4);
        double resU = res.getState()(0);
        double errU = sqrt(res.getCov()(0, 0));
        double resV = res.getState()(1);
        double errV = sqrt(res.getCov()(1, 1));

        if (NDF > 3) {
          m_hResidualU[avxdid]->Fill(resU, res.getWeight());
          m_hNormalizedResidualU[avxdid]->Fill(resU / errU, res.getWeight());

          m_hResidualV[avxdid]->Fill(resV, res.getWeight());
          m_hNormalizedResidualV[avxdid]->Fill(resV / errV, res.getWeight());

          m_hResidualUvsV[avxdid]->Fill(resU, v);
          m_hResidualVvsU[avxdid]->Fill(resV, u);
          m_hResidualUvsU[avxdid]->Fill(resU, u);
          m_hResidualVvsV[avxdid]->Fill(resV, v);

        }

        if (m_fillExpertHistos) {
          m_hNDFResidualU[avxdid]->Fill(NDF, resU, res.getWeight());
          m_hNDFNormalizedResidualU[avxdid]->Fill(NDF, resU / errU, res.getWeight());

          m_hNDFResidualV[avxdid]->Fill(NDF, resV, res.getWeight());
          m_hNDFNormalizedResidualV[avxdid]->Fill(NDF, resV / errV, res.getWeight());
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


  for (VxdID& avxdid : m_sensorList) {

    try {
      mop.extrapolateToPlane(genfit::SharedPlanePtr(new ROIDetPlane(m_vPlanes[avxdid])));
      if (m_vPlanes[avxdid].isInActive(mop.getPos(), mop.getMom())) {
        int layer = avxdid.getLayerNumber();
        m_hPseudoEfficienciesU->Fill(layer, 0);
        m_hPseudoEfficienciesV->Fill(layer, 0);
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

    //not sure if that makes sense !?!?! But as it was in the old code I will keep it.
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
