/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <arich/modules/arichDQM/ARICHDQMModule.h>

// ARICH
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHGeoDetectorPlane.h>
#include <arich/dbobjects/ARICHGeoAerogelPlane.h>
#include <arich/dataobjects/ARICHHit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHPhoton.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>

#include <TF1.h>
#include <TVector3.h>
#include <TDirectory.h>

#include <fstream>
#include <math.h>

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
    TDirectory* dirARICHDQM = oldDir->mkdir("ARICH");
    dirARICHDQM->cd();

    //Histograms for analysis and statistics

    h_chStat = new TH1D("chStat", "Status of channels;Channel serial;Status", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_aeroStat = new TH1D("aeroStat", "Status of aerogels;Aerogel tile serial;Status", 160, -0.5, 160 - 0.5);
    h_chHit = new TH1D("chHit", "Number of hits in each channel;Channel serial;Hits", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_chipHit = new TH1D("chipHit", "Number of hits in each chip;Chip serial;Hits", 420 * 4, -0.5, 420 * 4 - 0.5);
    h_hapdHit = new TH1D("hapdHit", "Number of hits in each HAPD;HAPD serial;Hits", 420, 0.5, 421 - 0.5);
    h_hapdHitPerEvent = new TH2D("hapdHitPerEvent", "Number of hits in each HAPD per Event;HAPD serial;Hits/event", 420, 0.5, 420 + 0.5,
                                 144, -0.5, 143.5);
    h_trackPerEvent = new TH1D("trackPerEvent", "Number of tracks in ARICH per event; # of tracks;Events", 6, -0.5, 5.5);

    h_mergerHit = new TH1D("mergerHit", "Number of hits in each merger board;MB ID;Hits", 72, 0.5, 72 + 0.5);
    h_bitsPerMergerNorm = new TH2D("bitsPerMergerNorm", "Normalised number of hits in each bit in each Merger;Bit;MB ID;Hits", 5,
                                   -1 - 0.5,
                                   4 - 0.5, 72, 0.5, 72 + 0.5); // copy of h_bits, normalised to number of connected Hapds and to sum(bit1, bit2)
    h_bitsPerHapdMerger = new TH2D("bitsPerHapdMerger",
                                   "Number of hits in each bit in each Hapd sorted by mergers;Bit;HAPD unsorted;Hits", 5, -1 - 0.5,
                                   4 - 0.5, 432, 1, 432);

    h_aerogelHit = new TH1D("aerogelHit", "Number track associated hits in each aerogel tile;Aerogel slot ID;Hits", 125, -0.5,
                            125 - 0.5);
    h_bits = new TH1D("bits", "Number of hits in each bit;Bit;Hits", 5, -1 - 0.5,
                      4 - 0.5); //Bin at -1 is added to set minimum 0 without SetMinimum(0) due to bugs in jsroot on DQM server.
    h_bitsPerChannel = new TH2D("bitsPerChannel", "Number of hits in each bit in each chanenel;Bit;channel #;Hits", 4, - 0.5,
                                4 - 0.5, 420 * 144, -0.5, 420 * 144 - 0.5); // copy of h_bits
    h_hitsPerTrack2D = new TH2D("hitsPerTrack2D", "2D distribution of track associated hits;X[cm];Y[cm];Hits", 230, -115, 115, 230,
                                -115, 115);
    h_tracks2D = new TH2D("tracks2D", "Distribution track positions;X[cm];Y[cm];Tracks", 230, -115, 115, 230, -115, 115);

    h_hitsPerEvent = new TH1D("hitsPerEvent", "Number of hit per event;Number of hits;Events", 150, -0.5, 150 - 0.5);
    h_theta = new TH1D("theta", "Cherenkov angle distribution;Angle [rad];Events", 60, 0, M_PI / 6);
    h_hitsPerTrack = new TH1D("hitsPerTrack", "Number of hit per track;Number of hits;Tracks", 150, -0.5, 150 - 0.5);

    for (int i = 0; i < 6; i++) {
      h_secTheta[i] = new TH1D(Form("thetaSec%d", i + 1), Form("Cherenkov angle distribution in sector %d;Angle [rad];Events", i + 1),
                               60, 0, M_PI / 6);
      h_secHitsPerTrack[i] = new TH1D(Form("hitsPerTrackSec%d", i + 1),
                                      Form("Number of hit per track in sector %d;Number of hits;Tracks", i + 1), 150, -0.5, 150 - 0.5);
      h_secHapdHit[i] = new TH1D(Form("hapdHit%d", i + 1), Form("Number of hits in each HAPD in sector %d;HAPD serial;Hits", i + 1), 70,
                                 0.5, 71 - 0.5);
    }

    h_chDigit = new TH1D("chDigit", "Number of raw digits in each channel;Channel serial;Hits", 420 * 144, -0.5, 420 * 144 - 0.5);
    h_chipDigit = new TH1D("chipDigit", "Number of raw digits in each chip;Chip serial;Hits", 420 * 4, -0.5, 420 * 4 - 0.5);
    h_hapdDigit = new TH1D("hapdDigit", "Number of raw digits in each HAPD;HAPD serial;Hits", 420, 0.5, 421 - 0.5);

    h_aerogelHits3D = new TH3D("aerogelHits3D", "Number of track associated hits for each aerogel tile; #phi section; r section", 125,
                               -0.5, 124.5, 20, 0, 20, 20, 0, 20);
    h_mirrorThetaPhi = new TH3D("mirrorThetaPhi",
                                "Cherenkov theta vs Cherenkov phi for mirror reflected photons; mirroID; #phi_{c} [rad]; #theta_{c} [rad]", 18, 0.5, 18.5, 100,
                                -M_PI, M_PI, 100, 0, 0.5);
    h_thetaPhi = new TH2D("thetaPhi", "Cherenkov theta vs phi;#phi [rad];#theta_{c} [rad]", 100, -M_PI, M_PI, 100, 0., 0.5);

    h_flashPerAPD = new TH1D("flashPerAPD", "Number of flashes per APD; APD serial; number of flash", 420 * 4, -0.5, 420 * 4 - 0.5);

    h_ARICHOccAfterInjLer = new TH1F("ARICHOccInjLER", " ARICHOccInjLER /Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    h_ARICHOccAfterInjHer = new TH1F("ARICHOccInjHER", " ARICHOccInjHER /Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    h_ARICHEOccAfterInjLer = new TH1F("ARICHEOccInjLER", "ARICHEOccInjLER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);
    h_ARICHEOccAfterInjHer = new TH1F("ARICHEOccInjHER", "ARICHEOccInjHER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);

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
    h_bitsPerMergerNorm->SetOption("LIVE");
    h_bitsPerHapdMerger->SetOption("LIVE");

    h_aerogelHit->SetOption("LIVE");
    h_bits->SetOption("LIVE");
    h_hitsPerTrack2D->SetOption("LIVE");
    h_tracks2D->SetOption("LIVE");

    h_hitsPerEvent->SetOption("LIVE");
    h_theta->SetOption("LIVE");
    h_hitsPerTrack->SetOption("LIVE");
    h_trackPerEvent->SetOption("LIVE");
    h_flashPerAPD->SetOption("LivE");

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->SetOption("LIVE");
      h_secHitsPerTrack[i]->SetOption("LIVE");
    }
    h_ARICHOccAfterInjLer->SetOption("LIVE");
    h_ARICHEOccAfterInjLer->SetOption("LIVE");
    h_ARICHOccAfterInjHer->SetOption("LIVE");
    h_ARICHEOccAfterInjHer->SetOption("LIVE");

    //Set the minimum to 0
    h_chDigit->SetMinimum(0);
    h_chipDigit->SetMinimum(0);
    h_hapdDigit->SetMinimum(0);
    h_chHit->SetMinimum(0);
    h_chipHit->SetMinimum(0);
    h_hapdHit->SetMinimum(0);
    h_mergerHit->SetMinimum(0);
    h_bitsPerMergerNorm->SetMinimum(0);
    h_bitsPerHapdMerger->SetMinimum(0);
    h_aerogelHit->SetMinimum(0);
    h_bits->SetMinimum(0);
    h_bitsPerChannel->SetMinimum(0);
    h_hitsPerTrack2D->SetMinimum(0);
    h_tracks2D->SetMinimum(0);
    h_flashPerAPD->SetMinimum(0);
    h_hitsPerEvent->SetMinimum(0);
    h_theta->SetMinimum(0);
    h_hitsPerTrack->SetMinimum(0);
    h_ARICHOccAfterInjLer->SetMinimum(0);
    h_ARICHEOccAfterInjLer->SetMinimum(0);
    h_ARICHOccAfterInjHer->SetMinimum(0);
    h_ARICHEOccAfterInjHer->SetMinimum(0);

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
    m_rawFTSW.isOptional(); /// better use isRequired(), but RawFTSW is not in sim
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
    h_bitsPerMergerNorm->Reset();
    h_bitsPerHapdMerger->Reset();

    h_aerogelHit->Reset();
    h_bits->Reset();
    h_bitsPerChannel->Reset();
    h_hitsPerTrack2D->Reset();
    h_tracks2D->Reset();
    h_aerogelHits3D->Reset();

    h_hitsPerEvent->Reset();
    h_theta->Reset();
    h_hitsPerTrack->Reset();
    h_trackPerEvent->Reset();
    h_flashPerAPD->Reset();
    h_mirrorThetaPhi->Reset();
    h_thetaPhi->Reset();

    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->Reset();
      h_secHitsPerTrack[i]->Reset();
    }

    h_ARICHOccAfterInjLer->Reset();
    h_ARICHEOccAfterInjLer->Reset();
    h_ARICHOccAfterInjHer->Reset();
    h_ARICHEOccAfterInjHer->Reset();
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

    std::vector<int> apds(420 * 4, 0);
    for (const auto& digit : arichDigits) {
      uint8_t bits = digit.getBitmap();
      int moduleID  = digit.getModuleID();
      int channelID = digit.getChannelID();
      int mergerID = arichMergerMap->getMergerID(moduleID);
      int febSlot = arichMergerMap->getFEBSlot(moduleID);
      unsigned binID = (mergerID - 1) * N_FEB2MERGER + (febSlot);

      for (int i = 0; i < 8; i++) {
        if ((bits & (1 << i)) && !(bits & ~(1 << i))) {
          h_bits->Fill(i);
          h_bitsPerChannel->Fill(i, (moduleID - 1) * 144 + channelID);
          h_bitsPerMergerNorm->Fill(i, mergerID);
          h_bitsPerHapdMerger->Fill(i, binID);
        } else if (!bits) {
          h_bits->Fill(8);
          h_bitsPerChannel->Fill(8, (moduleID - 1) * 144 + channelID);
          h_bitsPerMergerNorm->Fill(8, mergerID);
          h_bitsPerHapdMerger->Fill(8, binID);
        }
      }

      // fill occupancy histograms for raw data
      h_chDigit  ->Fill((moduleID - 1) * 144 + channelID);
      int chip = (moduleID - 1) * 4   + channelID / 36;
      h_chipDigit->Fill(chip);
      apds[chip] += 1;
      h_hapdDigit->Fill(moduleID);
    }

    int iapd = 0;
    for (auto apd : apds) { if (apd > 20) h_flashPerAPD->Fill(iapd); iapd++;}

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

    int ntrk = 0;
    for (const auto& arichTrack : arichTracks) {


      //Momentum limits are applied
      //if (arichTrack.getPhotons().size() == 0) continue;
      if (arichTrack.getMomentum() > 0.5) ntrk++; // count tracks with momentum larger than 0.5 GeV
      if (arichTrack.getMomentum() < m_momDnLim || arichTrack.getMomentum() > m_momUpLim) continue;

      TVector3 recPos = arichTrack.getPosition();
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

      std::vector<ARICHPhoton> photons = arichTrack.getPhotons();
      for (auto& photon : photons) {
        if (photon.getMirror() == 0) {
          if (trR < 93.) {
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
        } else {
          if (trR > 95.) h_mirrorThetaPhi->Fill(photon.getMirror(), photon.getPhiCer(), photon.getThetaCer());
        }
      }

      //Get ARICHLikelihood related to the ARICHTrack
      const ExtHit* extHit = arichTrack.getRelated<ExtHit>();
      const Track* mdstTrack = NULL;
      if (extHit) mdstTrack = extHit->getRelated<Track>();
      const ARICHLikelihood* lkh = NULL;
      if (mdstTrack) lkh = mdstTrack->getRelated<ARICHLikelihood>();
      else lkh = arichTrack.getRelated<ARICHLikelihood>();

      if (lkh) {
        if (!lkh->getFlag()) continue; //Fill only when the number of expected photons is more than 0.
        double nphoton = lkh->getDetPhot();
        h_hitsPerTrack->Fill(nphoton);
        h_secHitsPerTrack[trSector]->Fill(nphoton);
        h_hitsPerTrack2D->Fill(recPos.X(), recPos.Y(), nphoton);
        int aeroID = arichGeoAero.getAerogelTileID(recPos.X(), recPos.Y());
        h_aerogelHits3D->Fill(aeroID, (trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                              (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20),
                              nphoton);
        h_aerogelHit->Fill(aeroID, nphoton);
      }
    }

    h_trackPerEvent->Fill(ntrk);

    for (auto& it : m_rawFTSW) {
      B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
              (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
              it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));
      auto difference = it.GetTimeSinceLastInjection(0);
      if (difference != 0x7FFFFFFF) {
        unsigned int nentries = arichDigits.getEntries();
        float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
        if (it.GetIsHER(0)) {
          h_ARICHOccAfterInjHer->Fill(diff2, nentries);
          h_ARICHEOccAfterInjHer->Fill(diff2);
        } else {
          h_ARICHOccAfterInjLer->Fill(diff2, nentries);
          h_ARICHEOccAfterInjLer->Fill(diff2);
        }
      }
    }

  }

  void ARICHDQMModule::endRun()
  {

    DBObjPtr<ARICHMergerMapping> arichMergerMap;
    if (h_theta->GetEntries() < 200) return;
    TF1* f1 = new TF1("arichFitFunc", "gaus(0)+pol1(3)", 0.25, 0.4);
    f1->SetParameters(0.8 * h_theta->GetMaximum(), 0.323, 0.016, 0, 0);
    f1->SetParName(0, "C");
    f1->SetParName(1, "mean");
    f1->SetParName(2, "sigma");
    f1->SetParName(3, "p0");
    f1->SetParName(4, "p1");
    h_theta->Fit(f1, "R");

    //Normalise bins in histogram bitsPerMergerNorm
    for (int mergerID = 1; mergerID < 73; ++mergerID) {
      double NHapd = 0;
      for (int febSlot = 1; febSlot < 7; ++febSlot) {
        if (arichMergerMap->getModuleID(mergerID, febSlot) > 0) NHapd++;
      }

      double bin_value[5];
      for (int i = 1; i <= 5; ++i) {
        // loop over bits and save values
        bin_value[i - 1] = h_bitsPerMergerNorm->GetBinContent(i, mergerID);
      }

      for (int i = 1; i <= 5; ++i) {
        // loop over bits again and set bin content
        h_bitsPerMergerNorm->SetBinContent(i, mergerID,
                                           bin_value[i - 1] / (bin_value[3] + bin_value[2]) / NHapd); // normalise with sum of bit1 and bit2, and number of connected HAPDs
      }
    }


  }

  void ARICHDQMModule::terminate()
  {
  }
}
