/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Kindo Haruki                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichPhysicalDQM/ARICHPhysicalDQMModule.h>
#include <arich/modules/arichPhysicalDQM/newTHs.h>
#include <arich/modules/arichPhysicalDQM/hitMapMaker.h>

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

  REG_MODULE(ARICHPhysicalDQM);

  ARICHPhysicalDQMModule::ARICHPhysicalDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("Make summary of data quality.");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("OutputFileName", m_outputFileName, "Output file name", string("ARICHPhysicalDQMResult.root"));
    addParam("debug", m_debug, "debug mode", false);
  }

  ARICHPhysicalDQMModule::~ARICHPhysicalDQMModule()
  {
  }

  void ARICHPhysicalDQMModule::defineHisto()
  {

    /*
    TDirectory* oldDir = gDirectory;
    TDirectory* dirARICHPhys = NULL;
    dirARICHPhys = oldDir->mkdir("ARICHPhysDQM");
    dirARICHPhys->cd();
    */
    //Histograms for analysis and statistics

    h_chStat = newTH1("h_chStat", "Status of channels;# of channel;Status", 420 * 144, -0.5, 420 * 144 - 0.5, kRed, kRed);
    h_aeroStat = newTH1("h_aeroStat", "Status of aerogels;# of aerogel tile;Status", 160, -0.5, 160 - 0.5, kRed, kRed);

    h_chHit = newTH1("h_chHit", "# of hits in each channel;# of channel;Hits", 420 * 144, -0.5, 420 * 144 - 0.5, kRed, kRed);
    h_gelHit = newTH2("h_gelHit", "# of hits in each aerogel tile;Azimuth;Ring", 40, -0.5, 40 - 0.5, 4, 0.5, 4.5, kRed, kRed);
    h_hits2D = newTH2("h_hits2D", "Distribution of hits on its track position", 460, -115, 115, 460, -115, 115, kRed, kRed);
    h_tracks2D = newTH2("h_tracks2D", "Distribution track positions", 460, -115, 115, 460, -115, 115, kRed, kRed);

    h_hitsPerEvent = newTH1("h_hitsPerEvent", "# of hit per event;# of hits;Events", 150, 0, 150, kRed, kRed);
    h_theta = newTH1("h_theta", "Cherenkov angle distribution;Angle [rad];Events", 60, 0, M_PI / 6, kRed, kRed);
    h_hitsPerTrack = newTH1("h_hitsPerTrack", "# of hit per track;# of hits;Tracks", 40, 0, 40, kRed, kRed);

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

    for (int i = 0; i < 72; i++) {
      h_mergerHit[i] = newTH1(Form("h_mergerHit_%d", i), Form("# of hits in merger No%d;# of channel;# of hits", i), 144 * 6, -0.5,
                              144 * 6 - 0.5, kRed, kRed);
    }
  }

  void ARICHPhysicalDQMModule::initialize()
  {
    REG_HISTOGRAM
    StoreArray<MCParticle> MCParticles;
    MCParticles.isOptional();
    StoreArray<Track> tracks;
    tracks.isRequired();
    StoreArray<ARICHTrack> arichTracks;
    arichTracks.isRequired();
    StoreArray<ARICHAeroHit> arichAeroHits;
    arichAeroHits.isRequired();
    StoreArray<ARICHHit> arichHits;
    arichHits.isRequired();
    StoreArray<ARICHLikelihood> likelihoods;
    likelihoods.isRequired();
  }

  void ARICHPhysicalDQMModule::beginRun()
  {
  }

  void ARICHPhysicalDQMModule::event()
  {
    StoreArray<MCParticle> MCParticles;
    StoreArray<Track> tracks;
    StoreArray<ARICHHit> arichHits;
    StoreArray<ARICHTrack> arichTracks;
    StoreArray<ARICHAeroHit> arichAeroHits;
    StoreArray<ARICHLikelihood> arichLikelihoods;
    DBObjPtr<ARICHGeometryConfig> arichGeoConfig;
    const ARICHGeoDetectorPlane& arichGeoDec = arichGeoConfig->getDetectorPlane();
    const ARICHGeoAerogelPlane& arichGeoAero = arichGeoConfig->getAerogelPlane();

    if (arichHits.getEntries() == 0) return;

    int nHit = 0;
    for (int i = 0; i < arichHits.getEntries(); i++) {
      h_chHit->Fill((arichHits[i]->getModule() - 1) * 144 + (arichHits[i]->getChannel() - 1));
      nHit++;
    }
    h_hitsPerEvent->Fill(nHit);
    /*
    for(int i=0;i<72;i++){
      h_mergerHit[i]->Scale(0);
      h_mergerHit[i]->Add(mergerClusterHitMap1D(h_chHit,i));
    }*/

    for (const auto& arichLikelihood : arichLikelihoods) {

      ARICHTrack* arichTrack = arichLikelihood.getRelated<ARICHTrack>();

      if (arichTrack->getPhotons().size() == 0) continue;

      TVector3 recPos = arichTrack->getPosition();
      int sector = 0;
      double dPhi = recPos.Phi();
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
      for (int i = 0; i < (int)photons.size(); i++) {
        auto& photon(photons[i]);
        h_theta->Fill(photon.getThetaCer());
        h_secTheta[sector]->Fill(photon.getThetaCer());
        nPhoton++;

        int module = arichHits[photon.getHitID()]->getModule();
        int channel = arichHits[photon.getHitID()]->getChannel();
        h_chHit->Fill((module - 1) * 144 + (channel - 1));
      }
      h_hitsPerTrack->Fill(nPhoton);
      h_secHitsPerTrack[sector]->Fill(nPhoton);

      h_hits2D->Fill(recPos.X(), recPos.Y(), nPhoton);
      h_gelHit->Fill(iAzimuth, iRing, nPhoton);

      switch (iRing) {
        case 1:
          h_gelHits2D[iAzimuth]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                      (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                      nPhoton);
          h_gelTracks2D[iAzimuth]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                        (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          break;
        case 2:
          h_gelHits2D[iAzimuth + 22]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 22]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          break;
        case 3:
          h_gelHits2D[iAzimuth + 50]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 50]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          break;
        case 4:
          h_gelHits2D[iAzimuth + 84]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                           (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20) ,
                                           nPhoton);
          h_gelTracks2D[iAzimuth + 84]->Fill((trPhi - arichGeoAero.getRingDPhi(iRing)*iAzimuth) / (arichGeoAero.getRingDPhi(iRing) / 20) ,
                                             (trR - arichGeoAero.getRingRadius(iRing)) / ((arichGeoAero.getRingRadius(iRing + 1) - arichGeoAero.getRingRadius(iRing)) / 20));
          break;
        default:
          break;
      }
    }

  }

  void ARICHPhysicalDQMModule::endRun()
  {
  }

  void ARICHPhysicalDQMModule::terminate()
  {

    results = new TFile(m_outputFileName.c_str(), "recreate");

    //h_hits2D->Divide(h_tracks2D);

    h_chHit->Write();
    h_gelHit->Write();
    h_hits2D->Write();
    h_tracks2D->Write();
    for (int i = 0; i < 124; i++) {
      h_gelHits2D[i]->Write();
      h_gelTracks2D[i]->Write();
    }
    //for(int i=0;i<72;i++){
    //  h_mergerHit[i]->Write();
    //}

    h_hitsPerEvent->Write();
    h_theta->Write();
    h_hitsPerTrack->Write();
    for (int i = 0; i < 6; i++) {
      h_secTheta[i]->Write();
      h_secHitsPerTrack[i]->Write();
    }

    results->Close();
  }
}


