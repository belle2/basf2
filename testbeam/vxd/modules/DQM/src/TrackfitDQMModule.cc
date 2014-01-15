#include "testbeam/vxd/modules/DQM/TrackfitDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/reconstruction/SVDRecoHit.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
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
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  m_hNTracks = getHist("hNTracks", "number of tracks with property", 3, 0, 3);
  m_hNTracks->GetXaxis()->SetBinLabel(1, "fitted, converged");
  m_hNTracks->GetXaxis()->SetBinLabel(2, "fitted, not converged");
  m_hNTracks->GetXaxis()->SetBinLabel(3, "TrackCand, but no Track");
  m_hNDF = getHist("hNDF", "NDF of fitted track;NDF;Tracks", 15, -5, 10);
  m_hChi2 = getHist("hChi2", "#chi^{2} of tracks;#chi^{2};Tracks", 1000, 0, 20);
  m_hPval = getHist("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);
  for (int i = 0; i < c_nSVDPlanes; ++i) {
    int iLayer = indexToPlane(i);
    std::string title, name;
    name = (boost::format("hResidualsSVDU%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along U in layer %1%;cm;Tracks") % iLayer).str();
    m_hResidualSVDU[i] = getHist(name, title, 1000, -2, 2);
    name = (boost::format("hResidualsSVDV%1%") % iLayer).str();
    title = (boost::format("unnormalized, unbiased residuals along V in layer %1%;cm;Tracks") % iLayer).str();
    m_hResidualSVDV[i] = getHist(name, title, 1000, -2, 2);
    name = (boost::format("hNormalizedResidualsSVDU%1%") % iLayer).str();
    title = (boost::format("normalized, unbiased residuals in layer %1%;#sigma;Tracks") % iLayer).str();
    m_hNormalizedResidualSVDU[i] = getHist(name, title, 1000, -2, 2);
    name = (boost::format("hNormalizedResidualsSVDV%1%") % iLayer).str();
    title = (boost::format("normalized, unbiased residuals in layer %1%;#sigma;Tracks") % iLayer).str();
    m_hNormalizedResidualSVDV[i] = getHist(name, title, 1000, -2, 2);
  }

#if 0
  for (int i = 0; i < 5; ++i) {
    const char* varName[5] = { "u", "v", "u'", "v'", "q/p", };
    double lower[5] = { -3., -3., -.1, -.1, -1., };
    double upper[5] = {  3.,  3.,  .1,  .1,  1., };

    std::string name, title;
    name = (boost::format("hSeedQuality%1%") % i).str();
    title = (boost::format("seed vs. trackfit result for %1%") % varName[i]).str();
    //m_hSeedQuality[i] = getHist(name, title, 100, lower[i], upper[i], 100, lower[i], upper[i]);
  }
#endif
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
    m_hChi2->Fill(fs->getChi2());
    m_hNDF->Fill(fs->getNdf());
    m_hPval->Fill(fs->getPVal());
    // Only consider tracks fit with enough NDFs.  Unbiases residuals
    // don't make much sense otherwise.
    if (fs->getNdf() < 3)
      continue;
    for (unsigned int i = 0; i < track->getNumPoints(); ++i) {
      const genfit::TrackPoint* tp = track->getPoint(i);
      if (!tp->hasRawMeasurements())
        continue;

      // Identify Layer.
      const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
      const SVDRecoHit* rawSVD = dynamic_cast<const SVDRecoHit*>(raw);
      if (!rawSVD)
        continue;

      int layer = rawSVD->getSensorID().getLayerNumber();
      int index = planeToIndex(layer);

      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi)
        continue;

      unsigned int nMeasurements = kfi->getNumMeasurements();
      for (unsigned int iMeas = 0; iMeas < nMeasurements; ++iMeas) {
        const genfit::MeasurementOnPlane& res = kfi->getResidual(iMeas, false);
        const genfit::AbsHMatrix* h = kfi->getMeasurementOnPlane(iMeas)->getHMatrix();
        if (dynamic_cast<const genfit::HMatrixU*>(h)) {
          m_hResidualSVDU[index]->Fill(res.getState()(0), res.getWeight());
          m_hNormalizedResidualSVDU[index]->Fill(res.getState()(0) / sqrt(res.getCov()(0, 0)), res.getWeight());
        } else if (dynamic_cast<const genfit::HMatrixV*>(h)) {
          m_hResidualSVDV[index]->Fill(res.getState()(0), res.getWeight());
          m_hNormalizedResidualSVDV[index]->Fill(res.getState()(0) / sqrt(res.getCov()(0, 0)), res.getWeight());
        }
      }
    }
  }
}


void TrackfitDQMModule::endRun()
{
}


void TrackfitDQMModule::terminate()
{
}

const SVD::SensorInfo& TrackfitDQMModule::getInfo(int index) const
{
  int iPlane = indexToPlane(index);
  VxdID sensorID(iPlane, 1, iPlane);
  return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
}

