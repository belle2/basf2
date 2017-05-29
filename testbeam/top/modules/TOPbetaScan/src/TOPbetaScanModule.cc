/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPbetaScan/TOPbetaScanModule.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>
#include <top/geometry/TOPGeometryPar.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

#include <sstream>

// ROOT
#include <TRandom3.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPbetaScan)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPbetaScanModule::TOPbetaScanModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Reconstructs particle beta using extended maximum likelihood");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("betaScan.root"));
    addParam("betaMin", m_betaMin, "lower limit of beta range to scan", 0.9);
    addParam("betaMax", m_betaMax, "upper limit of beta range to scan", 1.1);
    addParam("numPoints", m_numPoints, "number of scan points", 20);
    addParam("numBisect", m_numBisect, "number of bisection steps", 10);
    addParam("numBins", m_numBins, "number of histogram bins", 200);
    addParam("numScanHistograms", m_numScanHistograms,
             "number of scan histograms to be written for control", 10);
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per bar", 0.0);
    addParam("scaleN0", m_scaleN0, "Scale factor for N0", 1.0);
    addParam("electronicJitter", m_electronicJitter,
             "r.m.s of electronic jitter [ns] - not used!", 50e-3);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use default one)", 0.0);
    addParam("everyNth", m_everyNth, "randomly choose every Nth event", 0);

    // initialize other private data members
    m_file = NULL;
    m_betaHistogram = NULL;

  }

  TOPbetaScanModule::~TOPbetaScanModule()
  {
  }

  void TOPbetaScanModule::initialize()
  {
    if (m_outputFileName.empty()) B2ERROR("TOPbetaScan: no output file name given");

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    m_betaHistogram = new TH1F("Hbeta", "reconstructed particle beta",
                               m_numBins, m_betaMin, m_betaMax);

    TOP::TOPconfigure config;
    config.print();

  }

  void TOPbetaScanModule::beginRun()
  {
  }

  void TOPbetaScanModule::event()
  {

    // randomly choose every Nth event

    if (m_everyNth > 0) {
      if (int(gRandom->Rndm() * m_everyNth) != 0) return;
    }

    // create reconstruction object

    double mass[1] = {Const::electron.getMass()}; // one hypothesis with electron mass
    TOP::TOPreco reco(1, mass, m_minBkgPerBar, m_scaleN0);
    if (m_maxTime > 0) reco.setTmax(m_maxTime);
    reco.clearData();

    // put photon hits into it

    StoreArray<TOPDigit> topDigits;
    int nHits = topDigits.getEntries();
    for (int i = 0; i < nHits; ++i) {
      TOPDigit* data = topDigits[i];
      reco.addData(data->getModuleID(), data->getPixelID(), data->getTime());
    }

    // collect extrapolated tracks

    std::vector<TOP::TOPtrack> tracks;
    getTracks(tracks, Const::pion);
    if (tracks.empty()) {
      B2WARNING("TOPbetaScan: no extrapoated tracks found");
      return;
    }

    // reconstruct beta track-by-track and histogram it

    for (unsigned int i = 0; i < tracks.size(); i++) {
      double beta = reconstructBeta(reco, tracks[i]);
      m_betaHistogram->Fill(beta);
    }

  }


  void TOPbetaScanModule::endRun()
  {
  }

  void TOPbetaScanModule::terminate()
  {

    m_file->cd();
    m_betaHistogram->Write();
    for (unsigned i = 0; i < m_scanHistograms.size(); i++) m_scanHistograms[i]->Write();
    m_file->Close();

  }

  void TOPbetaScanModule::printModuleParams() const
  {
  }

  void TOPbetaScanModule::getTracks(std::vector<TOP::TOPtrack>& tracks,
                                    Const::ChargedStable chargedStable)
  {
    Const::EDetector myDetID = Const::EDetector::TOP; // TOP
    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    int NumBars = geo->getNumModules();
    int pdgCode = abs(chargedStable.getPDGCode());
    double mass = chargedStable.getMass();

    StoreArray<Track> Tracks;

    for (int itra = 0; itra < Tracks.getEntries(); ++itra) {
      const Track* track = Tracks[itra];
      const TrackFitResult* fitResult = track->getTrackFitResult(chargedStable);
      if (!fitResult) {
        B2ERROR("No TrackFitResult for " << chargedStable.getPDGCode());
        continue;
      }
      int charge = fitResult->getChargeSign();

      RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdgCode) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() == 0 || extHit->getCopyID() > NumBars) continue;
        if (extHit->getStatus() != EXT_ENTER) continue;
        TVector3 point = extHit->getPosition();
        double x = point.X();
        double y = point.Y();
        double z = point.Z();
        TVector3 momentum = extHit->getMomentum();
        double px = momentum.X();
        double py = momentum.Y();
        double pz = momentum.Z();
        TOP::TOPtrack trk(x, y, z, px, py, pz, 0.0, charge);
        double tof = extHit->getTOF();
        trk.setTrackLength(tof, mass);
        tracks.push_back(trk);
      }
    }
  }


  double TOPbetaScanModule::reconstructBeta(TOP::TOPreco& reco,
                                            TOP::TOPtrack& track)
  {

    // array to hold scan values
    std::vector<double> logL(m_numPoints);

    // array to hold beta values
    std::vector<double> beta(m_numPoints);
    double Dbeta = (m_betaMax - m_betaMin) / m_numPoints;
    beta[0] = m_betaMin + Dbeta / 2;
    for (int i = 1; i < m_numPoints; i++) beta[i] = beta[i - 1] + Dbeta;

    // scan beta

    for (int i = 0; i < m_numPoints; i++) {
      logL[i] = getLogLikelihood(reco, track, beta[i]);
    }

    // find maximum

    int i0 = 0;
    double maxL = logL[i0];
    for (int i = 1; i < m_numPoints; i++) {
      if (logL[i] > maxL) {maxL = logL[i]; i0 = i;}
    }
    if (i0 == 0) i0++;
    if (i0 == m_numPoints - 1) i0--;

    double Beta[] = {beta[i0 - 1], 0, beta[i0], 0, beta[i0 + 1]};
    double LogL[] = {logL[i0 - 1], 0, logL[i0], 0, logL[i0 + 1]};
    for (int i = 0; i < m_numBisect; i++) {
      improvePrecision(reco, track, Beta, LogL);
    }

    // save scan to a histogram (for control)

    if ((int) m_scanHistograms.size() < m_numScanHistograms) {
      string id = string("Hscan") + numberToString(m_scanHistograms.size());
      StoreObjPtr<EventMetaData> evtMetaData;
      string htit = string("beta scan of event ") +
                    numberToString(evtMetaData->getEvent()) +
                    string(" run ") + numberToString(evtMetaData->getRun()) +
                    string(", ") +
                    numberToString(reco.getDataSize()) + string(" photons");
      TH1F* h = new TH1F(id.c_str(), htit.c_str(),
                         m_numPoints, m_betaMin, m_betaMax);
      for (int i = 0; i < m_numPoints; i++) h->SetBinContent(i + 1, logL[i] - LogL[2]);
      m_scanHistograms.push_back(h);
    }

    return Beta[2] + (Beta[2] - Beta[0]) * (gRandom->Rndm() - 0.5);

  }


  double TOPbetaScanModule::getLogLikelihood(TOP::TOPreco& reco,
                                             TOP::TOPtrack& track,
                                             double beta)
  {
    reco.setBeta(beta);
    reco.reconstruct(track);
    if (reco.getFlag() != 1) B2WARNING("TOPbetaScan: reconstruction flag = false");
    double logL[1], expPhot[1];
    int Nphot;
    reco.getLogL(1, logL, expPhot, Nphot);
    return logL[0];
  }


  void TOPbetaScanModule::improvePrecision(TOP::TOPreco& reco, TOP::TOPtrack& track,
                                           double Beta[], double LogL[])
  {
    Beta[1] = (Beta[0] + Beta[2]) / 2;
    LogL[1] = getLogLikelihood(reco, track, Beta[1]);

    Beta[3] = (Beta[2] + Beta[4]) / 2;
    LogL[3] = getLogLikelihood(reco, track, Beta[3]);

    int i0 = 1;
    double maxL = LogL[i0];
    for (int i = 2; i < 4; i++) {
      if (LogL[i] > maxL) {maxL = LogL[i]; i0 = i;}
    }

    switch (i0) {
      case 1:
        Beta[4] = Beta[2];
        LogL[4] = LogL[2];
        Beta[2] = Beta[1];
        LogL[2] = LogL[1];
        break;
      case 2:
        Beta[4] = Beta[3];
        LogL[4] = LogL[3];
        Beta[0] = Beta[1];
        LogL[0] = LogL[1];
        break;
      case 3:
        Beta[0] = Beta[2];
        LogL[0] = LogL[2];
        Beta[2] = Beta[3];
        LogL[2] = LogL[3];
        break;
      default:
        B2ERROR("improvePrecision: bug!");
    }
    Beta[1] = 0;
    LogL[1] = 0;
    Beta[3] = 0;
    LogL[3] = 0;

  }


  std::string TOPbetaScanModule::numberToString(int number)
  {
    stringstream ss;
    string str;
    ss << number;
    ss >> str;
    return str;
  }


} // end Belle2 namespace

