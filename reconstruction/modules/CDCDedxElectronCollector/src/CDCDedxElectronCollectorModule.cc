/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxElectronCollector/CDCDedxElectronCollectorModule.h>
#include <TTree.h>
#include <TH1F.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxElectronCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxElectronCollectorModule::CDCDedxElectronCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx electron calibrations");

  // Parameter definitions
  addParam("cleanupCuts", m_cuts, "Boolean to apply cleanup cuts", true);
  addParam("momentumCor", m_momCor, "Boolean to apply momentum correction", false);
  addParam("momentumCorFromDB", m_useDBMomCor, "Boolean to apply DB momentum correction", false);
  addParam("scaleCor", m_scaleCor, "Boolean to apply scale correction", false);
  addParam("cosineCor", m_cosineCor, "Boolean to apply cosine correction", false);
  addParam("maxNumHits", m_maxNumHits,
           "Maximum number of hits per track. If there is more than this the track will not be collected. ", int(100));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::prepare()
{
  // required input
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();

  m_eventMetaData.isRequired();

  // Data object creation
  auto means = new TH1F("means", "CDC dE/dx truncated means", 100, 0, 2);
  auto ttree = new TTree("tree", "Tree with dE/dx information");

  ttree->Branch<double>("dedx", &m_dedx);
  ttree->Branch<double>("costh", &m_costh);
  ttree->Branch<double>("p", &m_p);

  ttree->Branch("wire", &m_wire);
  ttree->Branch("layer", &m_layer);
  ttree->Branch("doca", &m_doca);
  ttree->Branch("enta", &m_enta);
  ttree->Branch("dedxhit", &m_dedxhit);

  // Collector object registration
  registerObject<TH1F>("means", means);
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::collect()
{
  int run = m_eventMetaData->getRun();

  //low run #
  const float momcorrlo[50] = {
    0.936667, 0.791667, 0.763456, 0.755219, 0.758876,
    0.762439, 0.769009, 0.776787, 0.783874, 0.791462,
    0.796567, 0.80445,  0.809177, 0.815605, 0.817414,
    0.822127, 0.828355, 0.83215,  0.832959, 0.833546,
    0.840324, 0.844323, 0.847539, 0.849506, 0.850848,
    0.852272, 0.854783, 0.853612, 0.861432, 0.859428,
    0.859533, 0.862021, 0.865721, 0.868412, 0.868954,
    0.872075, 0.872732, 0.872475, 0.872152, 0.876957,
    0.87419,  0.875742, 0.874523, 0.878218, 0.873543,
    0.881054, 0.874919, 0.877849, 0.886954, 0.882283
  };

  //high run #
  const float momcorrhi[50] = {
    1.14045,  0.73178,  0.709983, 0.711266, 0.716683,
    0.727419, 0.735754, 0.74534,  0.754149, 0.761252,
    0.768799, 0.77552,  0.780306, 0.786253, 0.79139,
    0.797053, 0.800905, 0.804441, 0.807102, 0.809439,
    0.815215, 0.818581, 0.821492, 0.823083, 0.824502,
    0.828764, 0.830907, 0.831392, 0.832376, 0.833232,
    0.836063, 0.839065, 0.841527, 0.84118,  0.842779,
    0.840801, 0.844476, 0.846664, 0.848733, 0.844318,
    0.84837,  0.850549, 0.852183, 0.851242, 0.856488,
    0.852705, 0.851871, 0.852278, 0.856854, 0.856848
  };

  // momentum correction
  float momcor[50];
  for (int i = 0; i < 50; ++i) {
    if (run <  3500) momcor[i] = momcorrlo[i];
    if (run >= 3500) momcor[i] = momcorrhi[i];
  }

  // Collector object access
  auto means = getObjectPtr<TH1F>("means");
  auto tree = getObjectPtr<TTree>("tree");

  for (int idedx = 0; idedx < m_dedxTracks.getEntries(); idedx++) {
    CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];
    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track) {
      B2WARNING("No related track...");
      continue;
    }
    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::electron);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    m_p = dedxTrack->getMomentum();
    TVector3 trackMom = fitResult->getMomentum();

    // apply Roy's cuts
    if (m_cuts && (fabs(m_p) >= 10.0 || fabs(m_p) <= 1.0)) continue;
    if (m_cuts && (dedxTrack->getNLayerHits() <= 42 || dedxTrack->getNLayerHits() >= 65)) continue;
    if (m_cuts && (trackMom.Phi() >= 0 || fitResult->getD0() >= 5 || fabs(fitResult->getZ0() - 35) >= 50)) continue;

    // determine the correction factor, if any
    double correction = 1.0;

    // determine Roy's corrections
    int mombin = 5.0 * fabs(m_p);
    if (m_momCor) {
      if (m_useDBMomCor) correction *= m_DBMomentumCor->getMean(fabs(m_p));
      else correction *= momcor[mombin];
    }

    // apply the scale factor
    if (m_scaleCor) correction *= m_DBScaleFactor->getScaleFactor();

    // apply the cosine corection
    double costh = dedxTrack->getCosTheta();
    if (m_cosineCor) {
      B2INFO("APPLYING COSING CORRECTION");
      correction *= m_DBCosineCor->getMean(costh);
    }

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_doca.clear();
    m_enta.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    if (correction == 0) continue;
    m_dedx = dedxTrack->getTruncatedMean() / correction;
    m_costh = costh;
    m_nhits = dedxTrack->size();

    if (m_nhits > m_maxNumHits) continue;
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(dedxTrack->getWire(i));
      m_layer.push_back(dedxTrack->getHitLayer(i));
      m_doca.push_back(dedxTrack->getDoca(i));
      m_enta.push_back(dedxTrack->getEnta(i));
      m_dedxhit.push_back(dedxTrack->getDedx(i) / correction);
    }

    // Track information filled
    tree->Fill();
    means->Fill(m_dedx);
  }
}
