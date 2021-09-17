/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/bklmAna/BKLMAnaModule.h>

/* Belle 2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMAna)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMAnaModule::BKLMAnaModule() : Module(),
  m_file(nullptr),
  m_extTree(nullptr),
  m_run(0),
  m_nExtHit(0),
  m_hdistance(nullptr),
  m_totalMom(nullptr),
  m_passMom(nullptr),
  m_effiMom(nullptr),
  m_totalYX(nullptr),
  m_passYX(nullptr),
  m_effiYX(nullptr),
  m_totalYZ(nullptr),
  m_passYZ(nullptr),
  m_effiYZ(nullptr)
{
  for (int i = 0; i < 200; ++i) {
    m_extx[i] = 0.0;
    m_exty[i] = 0.0;
    m_extz[i] = 0.0;
  }
  for (int i = 0; i < 15; ++i) {
    m_totalThephi[i] = nullptr;
    m_passThephi[i] = nullptr;
    m_effiThephi[i] = nullptr;
    m_totalTrkThephi[i] = nullptr;
    m_passTrkThephi[i] = nullptr;
    m_effiTrkThephi[i] = nullptr;
  }
  setDescription("analyze bklm efficiency associated to CDC, check performance of bklm et al.");
  addParam("filename", m_filename, "Output root filename", string("bklmana.root"));
}

BKLMAnaModule::~BKLMAnaModule()
{
}

void BKLMAnaModule::initialize()
{
  hits2D.isRequired();
  extHits.isRequired();
  tracks.isRequired();


  m_file = new TFile(m_filename.c_str(), "RECREATE");

  m_extTree = new TTree("exthit", "ext hit");
  m_extTree->Branch("run", &m_run, "m_run/I");
  m_extTree->Branch("nExtHit", &m_nExtHit, "m_nExtHit/I");
  m_extTree->Branch("x", &m_extx, "m_extx[m_nExtHit]/F");
  m_extTree->Branch("y", &m_exty, "m_exty[m_nExtHit]/F");
  m_extTree->Branch("z", &m_extz, "m_extz[m_nExtHit]/F");

  float phiBins = 36;
  float phiMin = 0.0;
  float phiMax = 360.0;
  float thetaBins = 19;
  float thetaMin = 35.0;
  float thetaMax = 130.0;
  char hname[50];
  for (int iL = 0; iL < 15 ; iL ++) {
    //based on theta phi of ExtHit position
    sprintf(hname, "denominator_Layer%i", iL + 1);
    m_totalThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    m_totalThephi[iL]->GetXaxis()->SetTitle("phi");
    m_totalThephi[iL]->GetYaxis()->SetTitle("theta");
    sprintf(hname, "numerator_Layer%i", iL + 1);
    m_passThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    sprintf(hname, "effi_Layer%i", iL + 1);
    m_passThephi[iL]->GetXaxis()->SetTitle("phi");
    m_passThephi[iL]->GetYaxis()->SetTitle("theta");
    m_effiThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    m_effiThephi[iL]->GetXaxis()->SetTitle("phi");
    m_effiThephi[iL]->GetYaxis()->SetTitle("theta");
    //based on theta phi of trk
    sprintf(hname, "Denominator_Layer%i", iL + 1);
    m_totalTrkThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    m_totalTrkThephi[iL]->GetXaxis()->SetTitle("trk.phi");
    m_totalTrkThephi[iL]->GetYaxis()->SetTitle("trk.theta");
    sprintf(hname, "Numerator_Layer%i", iL + 1);
    m_passTrkThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    sprintf(hname, "Effi_Layer%i", iL + 1);
    m_passTrkThephi[iL]->GetXaxis()->SetTitle("trk.phi");
    m_passTrkThephi[iL]->GetYaxis()->SetTitle("trk.theta");
    m_effiTrkThephi[iL] = new TH2F(hname, hname,  phiBins, phiMin, phiMax, thetaBins, thetaMin, thetaMax);
    m_effiTrkThephi[iL]->GetXaxis()->SetTitle("trk.phi");
    m_effiTrkThephi[iL]->GetYaxis()->SetTitle("trk.theta");
  }
  float gmin = -350;
  float gmax = 350;
  int gNbin = 150;
  float mommin = 0.;
  float mommax = 15;
  int mNbin = 30;

  m_hdistance = new TH1F("m_hdistance", " distance between mathced extHit and bklmHit2d ", 100, 0, 30);

  m_totalMom = new TH1F("m_totalMom", " denominator vs. p",  mNbin, mommin, mommax);
  m_passMom = new TH1F("m_passMom", " numerator vs. p",  mNbin, mommin, mommax);
  m_effiMom = new TH1F("m_effiMom", " effi. vs. p",  mNbin, mommin, mommax);
  m_effiMom->GetXaxis()->SetTitle("p (GeV)");
  m_effiMom->GetYaxis()->SetTitle("Efficiency (GeV)");

  m_totalYX  = new TH2F("m_totalYX", " denominator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYX  = new TH2F("m_passYX", " numerator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_totalYZ  = new TH2F("m_totalYZ", " denominator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYZ  = new TH2F("m_passYZ", " numerator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX  = new TH2F("m_effiYX", " effi. Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYZ  = new TH2F("m_effiYZ", " effi. Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX->GetXaxis()->SetTitle("x (cm)");
  m_effiYX->GetYaxis()->SetTitle("y (cm)");
  m_effiYZ->GetXaxis()->SetTitle("z (cm)");
  m_effiYZ->GetYaxis()->SetTitle("y (cm)");

}

void BKLMAnaModule::beginRun()
{
}

void BKLMAnaModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  //unsigned long eventNumber = eventMetaData->getEvent();
  unsigned long runNumber = eventMetaData->getRun();
  //unsigned long expNumber = eventMetaData->getExperiment();
  //StoreArray<RecoTrack> recoTracks;
  //numRecoTrk = recoTracks.getEntries();
  //StoreArray<BKLMTrack> bklmtracks;
  //StoreArray<TrackFitResult> trackFitResults;
  //StoreArray<MuidHit> muidHits;
  //StoreArray<Muid> muids;
  //StoreArray<MCParticle> mcParticles;

  //set<int> m_pointUsed;
  //m_pointUsed.clear();
  //check extHits
  //all ExtHit in bklm scope in each event, should not be many
  int nExtHit = 0;
  for (int t = 0; t < extHits.getEntries(); t++) {
    ExtHit* exthit =  extHits[t];
    if (exthit->getDetectorID() != Const::EDetector::BKLM)
      continue;
    m_extx[nExtHit] = exthit->getPosition()[0];
    m_exty[nExtHit] = exthit->getPosition()[1];
    m_extz[nExtHit] = exthit->getPosition()[2];
    nExtHit++;
    if (nExtHit > 199)
      break;
  }
  m_run = runNumber;
  m_nExtHit = nExtHit;

//the second way, require muid
  for (int k = 0; k < tracks.getEntries(); k++) {
    Track* track = tracks[k];
    // load the muon fit hypothesis or the hypothesis which is the clostes in mass to a muon
    // the tracking will not always fit a muon hypothesis
    const TrackFitResult* fitres = track->getTrackFitResultWithClosestMass(Belle2::Const::muon);
    double mom = fitres->getMomentum().Mag();
    //double  pt = fitres->getTransverseMomentum();
    B2Vector3D p3 = fitres->getMomentum();
    double trkphi = p3.Phi() * TMath::RadToDeg();
    double trktheta = p3.Theta() * TMath::RadToDeg();
    if (trkphi < 0)
      trkphi =  trkphi + 360.0;
    RelationVector<BKLMHit2d> relatedHit2D = track->getRelationsTo<BKLMHit2d>();
    RelationVector<ExtHit> relatedExtHit = track->getRelationsTo<ExtHit>();
    for (unsigned int t = 0; t < relatedExtHit.size(); t++) {
      ExtHit* exthit =  relatedExtHit[t];
      if (exthit->getDetectorID() != Const::EDetector::BKLM)
        continue;
      int module = exthit->getCopyID();
      int section = BKLMElementNumbers::getSectionByModule(module);
      int sector = BKLMElementNumbers::getSectorByModule(module);
      int layer = BKLMElementNumbers::getLayerByModule(module);
      bool crossed = false; // should be only once ?
      KLMMuidLikelihood* muid = track->getRelatedTo<KLMMuidLikelihood>();
      if (!muid)
        continue;
      int extPattern = muid->getExtLayerPattern();
      if ((extPattern & (1 << (layer - 1))) != 0)
        crossed = true;
      if (!crossed)
        continue;

      TVector3 extMom = exthit->getMomentum();
      TVector3 extVec = exthit->getPosition();
      bool matched = false;
      m_totalYX->Fill(extVec[0], extVec[1]);
      m_totalYZ->Fill(extVec[2], extVec[1]);
      float phi = extVec.Phi() * TMath::RadToDeg();
      float theta = extVec.Theta() * TMath::RadToDeg();
      if (phi < 0)
        phi =  phi + 360.0;
      m_totalThephi[layer - 1]->Fill(phi, theta);
      m_totalTrkThephi[layer - 1]->Fill(trkphi, trktheta);
      m_totalMom->Fill(mom);
      //look for mateched BKLM2dHit
      //for (unsigned int mHit = 0; mHit < relatedHit2D.size(); mHit++) {
      // BKLMHit2d* hit = relatedHit2D[mHit];
      for (int mHit = 0; mHit < hits2D.getEntries(); mHit++) {
        BKLMHit2d* hit = hits2D[mHit];
        //if(!hit->inRPC()) continue;
        if (hit->getSection() != section)
          continue;
        if (hit->getSector() != sector)
          continue;
        if (hit->getLayer() != layer)
          continue;
        TVector3 position = hit->getGlobalPosition();
        TVector3 distance =  extVec - position;
        //on same track, same sector, same layer, we should believe extHit and BKLMHit2d are matched.
        //let's record the distance to check, should be small
        m_hdistance->Fill(distance.Mag());
        if (distance.Mag() < 20)
          matched = true;
        //m_pointUsed.insert(m);
        if (matched)
          break;
      }
      if (matched) {
        m_passYX->Fill(extVec[0], extVec[1]);
        m_passYZ->Fill(extVec[2], extVec[1]);
        m_passTrkThephi[layer - 1]->Fill(trkphi, trktheta);
        m_passThephi[layer - 1]->Fill(phi, theta);
        m_passMom->Fill(mom);
      }
    }//end of loop ext hit
  }//end of loop tracks
  m_extTree->Fill();
}

void BKLMAnaModule::endRun()
{
}

void BKLMAnaModule::terminate()
{
  for (int iL = 0; iL < 15; iL ++) {
    for (int i = 0; i < m_totalThephi[iL]->GetNbinsX(); i++) {
      for (int j = 0; j < m_totalThephi[iL]->GetNbinsY(); j++) {
        float num = m_passThephi[iL]->GetBinContent(i + 1, j + 1);
        float denom = m_totalThephi[iL]->GetBinContent(i + 1, j + 1);
        if (num > 0) {
          m_effiThephi[iL]->SetBinContent(i + 1, j + 1, num / denom);
          m_effiThephi[iL]->SetBinError(i + 1, j + 1, sqrt(num * (denom - num) / (denom * denom * denom)));
        } else {
          m_effiThephi[iL]->SetBinContent(i + 1, j + 1, 0);
          m_effiThephi[iL]->SetBinError(i + 1, j + 1, 0);
        }
      }
    }
  }

  for (int iL = 0; iL < 15; iL ++) {
    for (int i = 0; i < m_totalTrkThephi[iL]->GetNbinsX(); i++) {
      for (int j = 0; j < m_totalTrkThephi[iL]->GetNbinsY(); j++) {
        float num = m_passTrkThephi[iL]->GetBinContent(i + 1, j + 1);
        float denom = m_totalTrkThephi[iL]->GetBinContent(i + 1, j + 1);
        if (num > 0) {
          m_effiTrkThephi[iL]->SetBinContent(i + 1, j + 1, num / denom);
          m_effiTrkThephi[iL]->SetBinError(i + 1, j + 1, sqrt(num * (denom - num) / (denom * denom * denom)));
        } else {
          m_effiTrkThephi[iL]->SetBinContent(i + 1, j + 1, 0);
          m_effiTrkThephi[iL]->SetBinError(i + 1, j + 1, 0);
        }
      }
    }
  }


  for (int i = 0; i < m_totalYX->GetNbinsX(); i++) {
    for (int j = 0; j < m_totalYX->GetNbinsY(); j++) {
      float num = m_passYX->GetBinContent(i + 1, j + 1);
      float denom = m_totalYX->GetBinContent(i + 1, j + 1);
      if (num > 0) {
        m_effiYX->SetBinContent(i + 1, j + 1, num / denom);
        m_effiYX->SetBinError(i + 1, j + 1, sqrt(num * (denom - num) / (denom * denom * denom)));
      } else {
        m_effiYX->SetBinContent(i + 1, j + 1, 0);
        m_effiYX->SetBinError(i + 1, j + 1, 0);
      }

      num = m_passYZ->GetBinContent(i + 1, j + 1);
      denom = m_totalYZ->GetBinContent(i + 1, j + 1);
      if (num > 0) {
        m_effiYZ->SetBinContent(i + 1, j + 1, num / denom);
        m_effiYZ->SetBinError(i + 1, j + 1, sqrt(num * (denom - num) / (denom * denom * denom)));
      } else {
        m_effiYZ->SetBinContent(i + 1, j + 1, 0);
        m_effiYZ->SetBinError(i + 1, j + 1, 0);
      }
    }
  }

  for (int i = 0; i < m_totalMom->GetNbinsX(); i++) {
    float num = m_passMom->GetBinContent(i + 1);
    float denom = m_totalMom->GetBinContent(i + 1);
    if (num > 0) {
      m_effiMom->SetBinContent(i + 1, num / denom);
      m_effiMom->SetBinError(i + 1, sqrt(num * (denom - num) / (denom * denom * denom)));
    } else {
      m_effiMom->SetBinContent(i + 1,  0);
      m_effiMom->SetBinError(i + 1,  0);
    }
  }

  m_file->cd();
  m_hdistance->Write();
  m_totalYX->Write();
  m_passYX->Write();
  m_totalYZ->Write();
  m_passYZ->Write();
  m_effiYX->Write();
  m_effiYZ->Write();
  m_totalMom->Write();
  m_passMom->Write();
  m_effiMom->Write();
  for (int i = 0; i < 15; i++) {
    m_totalThephi[i]->Write();
    m_passThephi[i]->Write();
    m_effiThephi[i]->Write();
    m_totalTrkThephi[i]->Write();
    m_passTrkThephi[i]->Write();
    m_effiTrkThephi[i]->Write();
  }
  m_extTree->Write();
  m_file->Close();
}

