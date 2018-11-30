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
#include <arich/modules/arichDQM/hitMapMaker.h>

// ARICH
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>
#include <arich/dbobjects/ARICHGeoDetectorPlane.h>
#include <arich/dbobjects/ARICHGeoAerogelPlane.h>
#include <framework/database/DBObjPtr.h>

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
    addParam("UpperMomentumLimit", m_momUpLim, "Upper momentum limit of tracks included in monitoring", 10.0);
    addParam("LowerMomentumLimit", m_momDnLim, "Lower momentum limit of tracks included in monitoring", 2.5);
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
    dirARICHDQM = oldDir->mkdir("ARICH");
    dirARICHDQM->cd();

    //Histograms for analysis and statistics

    h_chStat = new TH1D("chStat", "Status of channels;Channel serial;Status", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_aeroStat = new TH1D("aeroStat", "Status of aerogels;Aerogel tile serial;Status", 160, -0.5, 160 - 0.5);
    h_chHit = new TH1D("chHit", "Number of hits in each channel;Channel serial;Hits", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_chipHit = new TH1D("chipHit", "Number of hits in each chip;Chip serial;Hits", 420 * 4, -0.5, 420 * 4 - 0.5);
    h_hapdHit = new TH1D("hapdHit", "Number of hits in each HAPD;HAPD serial;Hits", 420, 0.5, 421 - 0.5);
    h_hapdHitPerEvent = new TH2D("hapdHitPerEvent", "Number of hits in each HAPD per Event;HAPD serial;Hits/event", 420, 0.5, 420 + 0.5,
                                 144, -0.5, 143.5);
    h_mergerHit = new TH1D("mergerHit", "Number of hits in each merger board;MB serial;Hits", 72, 0.5, 72 + 0.5);
    h_aerogelHit = new TH1D("aerogelHit", "Number track associated hits in each aerogel tile;Aerogel slot ID;Hits", 125, -0.5,
                            125 - 0.5);
    h_bits = new TH1D("bits", "Number of hits in each bit;Bit;Hits", 4, -0.5, 4 - 0.5);
    h_hitsPerTrack2D = new TH2D("hitsPerTrack2D", "2D distribution of track associated hits;X[cm];Y[cm];Hits", 230, -115, 115, 230,
                                -115, 115);
    h_tracks2D = new TH2D("tracks2D", "Distribution track positions;X[cm];Y[cm];Tracks", 230, -115, 115, 230, -115, 115);

    h_hitsPerEvent = new TH1D("hitsPerEvent", "Number of hit per event;Number of hits;Events", 150, -0.5, 150 - 0.5);
    h_theta = new TH1D("theta", "Cherenkov angle distribution;Angle [rad];Events", 60, 0, M_PI / 6);
    h_hitsPerTrack = new TH1D("hitsPerTrack", "Number of hit per track;Number of hits;Tracks", 41, -0.5, 40.5);

    for (int i = 0; i < 6; i++) {
      h_secTheta[i] = new TH1D(Form("thetaSec%d", i + 1), Form("Cherenkov angle distribution in sector %d;Angle [rad];Events", i + 1),
                               60, 0, M_PI / 6);
      h_secHitsPerTrack[i] = new TH1D(Form("hitsPerTrackSec%d", i + 1),
                                      Form("Number of hit per track in sector %d;Number of hits;Tracks", i + 1), 40, 0, 40);
      h_secHapdHit[i] = new TH1D(Form("hapdHit%d", i + 1), Form("Number of hits in each HAPD in sector %d;HAPD serial;Hits", i + 1), 70,
                                 0.5, 71 - 0.5);
    }

    TDirectory* dirAerogel = NULL;
    dirAerogel =  dirARICHDQM->mkdir("expert");
    dirAerogel->cd();

    h_chDigit = new TH1D("chDigit", "Number of raw digits in each channel;Channel serial;Hits", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_chipDigit = new TH1D("chipDigit", "Number of raw digits in each chip;Chip serial;Hits", 420 * 4, -0.5, 420 * 4 - 0.5);
    h_hapdDigit = new TH1D("hapdDigit", "Number of raw digits in each HAPD;HAPD serial;Hits", 420, 0.5, 421 - 0.5);

    h_aerogelHits3D = new TH3D("aerogelHits3D", "Number of track associated hits for each aerogel tile; #phi section; r section", 125,
                               -0.5, 124.5, 20, 0, 20, 20, 0, 20);
    h_mirrorThetaPhi = new TH3D("mirrorThetaPhi",
                                "Cherenkov theta vs Cherenkov phi for mirror reflected photons; mirroID; #phi_{c} [rad]; #theta_{c} [rad]", 18, 0.5, 18.5, 100,
                                -M_PI, M_PI, 100, 0, 0.5);
    h_thetaPhi = new TH2D("thetaPhi", "Cherenkov theta vs phi;#phi [rad];#theta_{c} [rad]", 100, -M_PI, M_PI, 100, 0., 0.5);

    dirARICHDQM->cd();

    //Select "LIVE" monitoring histograms
    h_chStat->SetOption("LIVE");
    h_aeroStat->SetOption("LIVE");

    h_chHit->SetOption("LIVE");
    h_chipHit->SetOption("LIVE");
    h_hapdHit->SetOption("LIVE");

    h_chDigit->SetOption("LIVE");
    h_chipDigit->SetOption("LIVE");
    h_hapdDigit->SetOption("LIVE");
    h_mergerHit->SetOption("LIVE");

    h_aerogelHit->SetOption("LIVE");
    h_bits->SetOption("LIVE");
    h_hitsPerTrack2D->SetOption("LIVE");
    h_tracks2D->SetOption("LIVE");

    h_hitsPerEvent->SetOption("LIVE");
    h_theta->SetOption("LIVE");
    h_hitsPerTrack->SetOption("LIVE");

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->SetOption("LIVE");
      h_secHitsPerTrack[i]->SetOption("LIVE");
    }

    //Set the minimum to 0
    h_chDigit->SetMinimum(0);
    h_chipDigit->SetMinimum(0);
    h_hapdDigit->SetMinimum(0);
    h_chHit->SetMinimum(0);
    h_chipHit->SetMinimum(0);
    h_hapdHit->SetMinimum(0);
    h_mergerHit->SetMinimum(0);
    h_aerogelHit->SetMinimum(0);
    h_bits->SetMinimum(0);
    h_hitsPerTrack2D->SetMinimum(0);
    h_tracks2D->SetMinimum(0);

    h_hitsPerEvent->SetMinimum(0);
    h_theta->SetMinimum(0);
    h_hitsPerTrack->SetMinimum(0);

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->SetMinimum(0);
      h_secHitsPerTrack[i]->SetMinimum(0);
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

  }

  void ARICHDQMModule::beginRun()
  {

    h_chStat->Reset();
    h_aeroStat->Reset();

    h_chDigit->Reset();
    h_chipDigit->Reset();
    h_hapdDigit->Reset();

    h_chHit->Reset();
    h_chipHit->Reset();
    h_hapdHit->Reset();
    h_mergerHit->Reset();

    h_aerogelHit->Reset();
    h_bits->Reset();
    h_hitsPerTrack2D->Reset();
    h_tracks2D->Reset();
    h_aerogelHits3D->Reset();

    h_hitsPerEvent->Reset();
    h_theta->Reset();
    h_hitsPerTrack->Reset();

    h_mirrorThetaPhi->Reset();
    h_thetaPhi->Reset();

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

    if (!arichLikelihoods.getEntries() && m_arichEvents) { setReturnValue(0); return;}

    for (const auto& digit : arichDigits) {
      uint8_t bits = digit.getBitmap();
      for (int i = 0; i < 8; i++) {
        if ((bits & (1 << i)) && !(bits & ~(1 << i))) h_bits->Fill(i);
        else if (!bits) h_bits->Fill(8);
      }
      // fill occupancy histograms for raw data
      int moduleID  = digit.getModuleID();
      int channelID = digit.getChannelID();
      h_chDigit  ->Fill((moduleID - 1) * 144 + channelID);
      h_chipDigit->Fill((moduleID - 1) * 4   + channelID / 36);
      h_hapdDigit->Fill(moduleID);
    }

    std::vector<int> hpd(420, 0);
    int nHit = 0;

    for (int i = 0; i < arichHits.getEntries(); i++) {

      int moduleID = arichHits[i]->getModule();
      int channelID = arichHits[i]->getChannel();
      h_chHit->Fill((moduleID - 1) * 144 + channelID);
      hpd[moduleID - 1]++;
      h_chipHit->Fill((moduleID - 1) * 4 + channelID / 36);
      h_hapdHit->Fill(moduleID);
      if (moduleID > 420) B2INFO("Invalid hapd number " << LogVar("hapd ID", moduleID));

      for (int j = 1; j <= 7; j++) {
        int ringStart = (j - 1) * (84 + (j - 2) * 6) / 2 + 1; // The smallest module ID in each ring
        int ringEnd = j * (84 + (j - 1) * 6) / 2; // The biggest module ID in each ring
        if (ringStart <= moduleID && moduleID <= ringEnd) {
          h_secHapdHit[(moduleID - ringStart) / (6 + j)]->Fill((moduleID - ringStart) % (6 + j) + 1 + (ringStart - 1) / 6);
        }
      }

      int mergerID = arichMergerMap->getMergerID(moduleID);
      h_mergerHit->Fill(mergerID);
      nHit++;
    }

    h_hitsPerEvent->Fill(nHit);
    int mmid = 1;
    for (auto hh : hpd) { h_hapdHitPerEvent->Fill(mmid, hh); mmid++;}

    for (const auto& arichLikelihood : arichLikelihoods) {

      ARICHTrack* arichTrack = arichLikelihood.getRelated<ARICHTrack>();

      //Momentum limits are applied
      if (arichTrack->getPhotons().size() == 0) continue;
      if (arichTrack->getMomentum() < m_momDnLim || arichTrack->getMomentum() > m_momUpLim) continue;

      TVector3 recPos = arichTrack->getPosition();
      int trSector = 0;
      double dPhi = 0;
      if (recPos.Phi() >= 0) {
        dPhi = recPos.Phi();
      } else {
        dPhi = 2 * M_PI + recPos.Phi();
      }
      while (dPhi > M_PI / 3 && trSector < 5) {
        dPhi -= M_PI / 3;
        trSector++;
      }
      double trR = recPos.XYvector().Mod();
      double trPhi = 0;
      if (recPos.Phi() >= 0) {
        trPhi = recPos.Phi();
      } else {
        trPhi = 2 * M_PI + recPos.Phi();
      }

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
          if (trR < 95.) {
            h_thetaPhi->Fill(photon.getPhiCer(), photon.getThetaCer());
            h_theta->Fill(photon.getThetaCer());
          }
          int hitSector = 0;
          double hitPhi = arichGeoDec.getSlotPhi(arichHits[photon.getHitID()]->getModule());
          if (hitPhi < 0) hitPhi += 2 * M_PI;
          while (hitPhi > M_PI / 3 && hitSector < 5) {
            hitPhi -= M_PI / 3;
            hitSector++;
          }
          h_secTheta[hitSector]->Fill(photon.getThetaCer());
          nPhoton++;
        } else {
          if (trR > 85.) h_mirrorThetaPhi->Fill(photon.getMirror(), photon.getPhiCer(), photon.getThetaCer());
        }
      }

      h_hitsPerTrack->Fill(nPhoton);
      h_secHitsPerTrack[trSector]->Fill(nPhoton);
      h_hitsPerTrack2D->Fill(recPos.X(), recPos.Y(), nPhoton);

      int aeroID = arichGeoAero.getAerogelTileID(recPos.X(), recPos.Y());
      h_aerogelHits3D->Fill(aeroID, (trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                            (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                            nPhoton);
      h_aerogelHit->Fill(aeroID, nPhoton);
    }

  }

  void ARICHDQMModule::endRun()
  {
  }

  void ARICHDQMModule::terminate()
  {
  }
}


