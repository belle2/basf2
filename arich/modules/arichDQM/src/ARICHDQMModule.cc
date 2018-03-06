/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki, Luka Santelj                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichDQM/ARICHDQMModule.h>
#include <arich/modules/arichDQM/newTHs.h>
#include <arich/modules/arichDQM/hitMapMaker.h>

// ARICH
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>
#include <arich/dbobjects/ARICHGeoDetectorPlane.h>
#include <arich/dbobjects/ARICHGeoAerogelPlane.h>
#include <framework/database/DBObjPtr.h>

#include <tracking/dataobjects/ExtHit.h>
#include <arich/dataobjects/ARICHHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHPhoton.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>

// Raw data object class
#include <rawdata/dataobjects/RawARICH.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TMath.h>
#include <THStack.h>
#include <TVector3.h>
#include <TFile.h>
#include <TImage.h>
#include <TPad.h>

#include <sstream>
#include <fstream>
#include <math.h>
#include <algorithm>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHDQM);

  ARICHDQMModule::ARICHDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("Make summary of data quality.");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("debug", m_debug, "debug mode", false);
    addParam("UpperMomentumLimit", m_momUpLim, "Upper momentum limit of tracks included in monitoring", 0.);
    addParam("LowerMomentumLimit", m_momDnLim, "Lower momentum limit of tracks included in monitoring", 0.);
    addParam("ArichEvents", m_arichEvents, "Include only hits from events where an extrapolated track to arich exists", false);
    addParam("MaxHits", m_maxHits, "Include only events with less than MaxHits hits in ARICH (remove loud events)", 70000);
    addParam("MinHits", m_minHits, "Include only events with more than MinHits hits in ARICH", 0);
  }

  ARICHDQMModule::~ARICHDQMModule()
  {
  }

  void ARICHDQMModule::defineHisto()
  {

    TDirectory* oldDir = gDirectory;
    TDirectory* dirARICHDQM = NULL;
    dirARICHDQM = oldDir->mkdir("ARICHDQM");
    dirARICHDQM->cd();

    //Histograms for analysis and statistics

    h_chStat = newTH1("h_chStat", "Status of channels;# of channel;Status", 420 * 144, -0.5, 420 * 144 - 0.5, kRed, kRed);
    h_aeroStat = newTH1("h_aeroStat", "Status of aerogels;# of aerogel tile;Status", 160, -0.5, 160 - 0.5, kRed, kRed);

    h_chHit = newTH1("h_chHit", "# of hits in each channel;Channel serial;Hits", 420 * 144, -0.5, 420 * 144 - 0.5, kRed, kRed);
    h_chipHit = newTH1("h_chipHit", "# of hits in each chip;Chip serial;Hits", 420 * 4, -0.5, 420 * 4 - 0.5, kRed, kRed);
    h_hapdHit = newTH1("h_hapdHit", "# of hits in each channel;HAPD serial;Hits", 420, 0.5, 421 - 0.5, kRed, kRed);
    h_hapdHitPerEvent = newTH2("h_hapdHitPerEvent", "# of hits in each HAPD per Event;HAPD serial;Hits/event", 420, 0.5, 420 + 0.5, 144,
                               -0.5, 143.5, kRed, kRed);
    h_mergerHit = newTH1("h_mergerHit", "# of hits in each merger board;MB serial;Hits", 72, 0.5, 72 + 0.5, kRed, kRed);
    h_gelHit = newTH1("h_gelHit", "# of hits in each aerogel tile;Aerogel slot ID;Hits", 124, -0.5, 124 - 0.5, kRed, kRed);
    h_bits = newTH1("h_bits", "# of hits in each bit;Bit;Hits", 4, -0.5, 4 - 0.5, kRed, kRed);
    h_hits2D = newTH2("h_hits2D", "Distribution of hits on its track position", 460, -115, 115, 460, -115, 115, kRed, kRed);
    h_tracks2D = newTH2("h_tracks2D", "Distribution track positions", 460, -115, 115, 460, -115, 115, kRed, kRed);

    h_hitsPerEvent = newTH1("h_hitsPerEvent", "# of hit per event;# of hits;Events", 150, 0, 150, kRed, kRed);
    h_theta = newTH1("h_theta", "Cherenkov angle distribution;Angle [rad];Events", 60, 0, M_PI / 6, kRed, kRed);
    h_hitsPerTrack = newTH1("h_hitsPerTrack", "# of hit per track;# of hits;Tracks", 41, -0.5, 40.5, kRed, kRed);

    for (int i = 0; i < 6; i++) {
      h_secTheta[i] = newTH1(Form("h_thetaSec%d", i), Form("Cherenkov angle distribution in sector %d;Angle [rad];Events", i), 60, 0,
                             M_PI / 6, kRed, kRed);
      h_secHitsPerTrack[i] = newTH1(Form("h_hitsPerTrackSec%d", i), Form("# of hit per track in sector %d;# of hits;Tracks", i), 40, 0,
                                    40, kRed, kRed);
    }

    for (int i = 0; i < 124; i++) {
      h_gelHits2D[i] = newTH2(Form("h_gelHits2d_%d", i), Form("Distribution of hits per track on aerogel tile coodinate No.%d", i), 20, 0,
                              20, 20, 0, 20, kRed, kRed);
      h_gelTracks2D[i] = newTH2(Form("h_gelTracks2d_%d", i), Form("Distribution of hits per track on aerogel tile coodinate No.%d", i),
                                20, 0, 20, 20, 0, 20, kRed, kRed);
    }

    //Select "LIVE" monitoring histograms
    h_chStat->SetOption("LIVE");
    h_aeroStat->SetOption("LIVE");

    h_chHit->SetOption("LIVE");
    h_chipHit->SetOption("LIVE");
    h_hapdHit->SetOption("LIVE");
    h_mergerHit->SetOption("LIVE");
    h_gelHit->SetOption("LIVE");
    h_bits->SetOption("LIVE");
    h_hits2D->SetOption("LIVE");
    h_tracks2D->SetOption("LIVE");

    h_hitsPerEvent->SetOption("LIVE");
    h_theta->SetOption("LIVE");
    h_hitsPerTrack->SetOption("LIVE");

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->SetOption("LIVE");
      h_secHitsPerTrack[i]->SetOption("LIVE");
    }

    oldDir->cd();
  }

  void ARICHDQMModule::initialize()
  {
    REG_HISTOGRAM
    StoreArray<MCParticle> MCParticles;
    MCParticles.isOptional();
    StoreArray<Track> tracks;
    tracks.isOptional();
    StoreArray<ARICHHit> arichHits;
    arichHits.isOptional();
    StoreArray<ARICHDigit> arichDigits;
    arichDigits.isRequired();
    StoreArray<ARICHTrack> arichTracks;
    arichTracks.isOptional();
    StoreArray<ARICHAeroHit> arichAeroHits;
    arichAeroHits.isOptional();
    StoreArray<ARICHLikelihood> likelihoods;
    likelihoods.isOptional();
    StoreArray<ExtHit> extHits;
    extHits.isOptional();

  }

  void ARICHDQMModule::beginRun()
  {

    h_chStat->Reset();
    h_aeroStat->Reset();

    h_chHit->Reset();
    h_chipHit->Reset();
    h_hapdHit->Reset();
    h_mergerHit->Reset();
    h_gelHit->Reset();
    h_bits->Reset();
    h_hits2D->Reset();
    h_tracks2D->Reset();
    for (int i = 0; i < 124; i++) {
      h_gelHits2D[i]->Reset();
      h_gelTracks2D[i]->Reset();
    }

    h_hitsPerEvent->Reset();
    h_theta->Reset();
    h_hitsPerTrack->Reset();

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->Reset();
      h_secHitsPerTrack[i]->Reset();
    }

  }

  void ARICHDQMModule::event()
  {
    StoreArray<MCParticle> MCParticles;
    StoreArray<Track> tracks;
    StoreArray<ARICHDigit> arichDigits;
    StoreArray<ARICHHit> arichHits;
    StoreArray<ARICHTrack> arichTracks;
    StoreArray<ARICHAeroHit> arichAeroHits;
    StoreArray<ARICHLikelihood> arichLikelihoods;
    DBObjPtr<ARICHGeometryConfig> arichGeoConfig;
    const ARICHGeoDetectorPlane& arichGeoDec = arichGeoConfig->getDetectorPlane();
    const ARICHGeoAerogelPlane& arichGeoAero = arichGeoConfig->getAerogelPlane();
    DBObjPtr<ARICHChannelMapping> arichChannelMap;
    DBObjPtr<ARICHMergerMapping> arichMergerMap;

    setReturnValue(1);

    if (arichHits.getEntries() < m_minHits || arichHits.getEntries() > m_maxHits) { setReturnValue(0); return;}


    Const::EDetector myDetID = Const::EDetector::ARICH; // arich
    StoreArray<ExtHit> extHits;
    int arichhit = 0;
    for (const auto& extHit : extHits) if (extHit.getDetectorID() == myDetID) arichhit = 1;
    if (!arichhit && extHits.getEntries() && m_arichEvents) { setReturnValue(0); return;}

    for (const auto& digit : arichDigits) {
      uint8_t bits = digit.getBitmap();
      for (int i = 0; i < 8; i++) {
        if (bits & (1 << i)) h_bits->Fill(i);
      }
    }
    std::vector<int> hpd(420, 0);
    int nHit = 0;
    for (int i = 0; i < arichHits.getEntries(); i++) {

      int moduleID = arichHits[i]->getModule();
      h_chHit->Fill((moduleID - 1) * 144 + arichHits[i]->getChannel());
      hpd[moduleID - 1]++;
      int x = 12 , y = 12;
      arichChannelMap->getXYFromAsic(arichHits[i]->getChannel(), x, y);
      if (x >= 12 || y >= 12) {
        B2INFO("Invalid channel position (x,y)=(" << x << "," << y << ").");
      } else {
        h_chipHit->Fill((moduleID - 1) * 4 + (x + 2 * y));
      }

      if (moduleID > 420) {
        B2INFO("Invalid hapd number " << moduleID);
      } else {
        h_hapdHit->Fill(moduleID);
      }

      int mergerID = arichMergerMap->getMergerID(moduleID);
      if (mergerID > 72) {
        B2INFO("Invalid MB number " << mergerID);
      } else {
        h_mergerHit->Fill(mergerID);
      }

      nHit++;
    }

    h_hitsPerEvent->Fill(nHit);
    int mmid = 1;
    for (auto hh : hpd) { h_hapdHitPerEvent->Fill(mmid, hh); mmid++;}

    for (const auto& arichLikelihood : arichLikelihoods) {

      ARICHTrack* arichTrack = arichLikelihood.getRelated<ARICHTrack>();

      //Momentum limits are applied
      if (arichTrack->getPhotons().size() == 0) continue;
      if (m_momUpLim + m_momDnLim != 0 && (arichTrack->getMomentum() < m_momDnLim || arichTrack->getMomentum() > m_momUpLim)) continue;

      TVector3 recPos = arichTrack->getPosition();
      int sector = 0;
      double dPhi = 0;
      if (recPos.Phi() >= 0) {
        dPhi = 2 * M_PI - recPos.Phi();
      } else {
        dPhi = -recPos.Phi();
      }
      while (dPhi > M_PI / 3 && sector < 6) {
        dPhi -= M_PI / 3;
        sector++;
      }
      double trR = recPos.XYvector().Mod();
      double trPhi = recPos.Phi() + M_PI;

      int iRing = 0;
      int iAzimuth = 0;
      while (arichGeoAero.getRingRadius(iRing + 1) < trR && iRing < 4) {
        iRing++;
      }
      if (iRing == 0) continue;
      while (arichGeoAero.getRingDPhi(iRing) * (iAzimuth + 1) < trPhi) {
        iAzimuth++;
      }

      h_tracks2D->Fill(recPos.X(), recPos.Y());

      std::vector<ARICHPhoton> photons = arichTrack->getPhotons();
      int nPhoton = 0;
      for (auto& photon : photons) {
        if (photon.getMirror() == 0) {
          h_theta->Fill(photon.getThetaCer());
          h_secTheta[sector]->Fill(photon.getThetaCer());
          nPhoton++;
        }
      }

      h_hitsPerTrack->Fill(nPhoton);
      h_secHitsPerTrack[sector]->Fill(nPhoton);

      h_hits2D->Fill(recPos.X(), recPos.Y(), nPhoton);

      switch (iRing) {
        case 1:
          h_gelHits2D[iAzimuth]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                      (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                      nPhoton);
          h_gelTracks2D[iAzimuth]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                        (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          h_gelHit->Fill(iAzimuth, nPhoton);
          break;
        case 2:
          h_gelHits2D[iAzimuth + 22]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 22]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          h_gelHit->Fill(iAzimuth + 22, nPhoton);
          break;
        case 3:
          h_gelHits2D[iAzimuth + 50]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 50]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          h_gelHit->Fill(iAzimuth + 50, nPhoton);
          break;
        case 4:
          h_gelHits2D[iAzimuth + 84]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 84]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          h_gelHit->Fill(iAzimuth + 84, nPhoton);
          break;
        default:
          break;
      }
    }

  }

  void ARICHDQMModule::endRun()
  {
  }

  void ARICHDQMModule::terminate()
  {
  }
}


