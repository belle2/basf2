/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmTracking/BKLMTrackingModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <bklm/geometry/GeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;
using namespace CLHEP;

REG_MODULE(BKLMTracking)

BKLMTrackingModule::BKLMTrackingModule() : Module()
{
  setDescription("perform standard-alone straight line tracking for BKLM");
  addParam("MatchToRecoTrack", m_MatchToRecoTrack, "[bool], whether match BKLMTrack to RecoTrack; (default is false)", false);
  addParam("MaxAngleRequired", m_maxAngleRequired,
           "[degree], match BKLMTrack to RecoTrack; angle between them is required to be smaller than (default 10)", double(10.0));
  addParam("fitGlobalBKLMTrack", m_globalFit,
           "[bool], do the BKLMTrack fitting in global system (multi-sectors track) or local system (sector by sector) (default is false, local sys.)",
           false);
  addParam("StudyEffiMode", m_studyEffi, "[bool], run in efficieny study mode (default is false)", false);
  addParam("outputName", m_outPath , "[string],  output file name containing efficiencies plots ", string("bklmEffi.root"));
}

BKLMTrackingModule::~BKLMTrackingModule()
{

}

void BKLMTrackingModule::initialize()
{

  hits2D.isRequired();
  m_storeTracks.registerInDataStore();
  m_storeTracks.registerRelationTo(hits2D);
  m_storeTracks.registerRelationTo(recoTracks);
  recoHitInformation.registerRelationTo(hits2D);
  hits2D.registerRelationTo(recoTracks);

  m_file =     new TFile(m_outPath.c_str(), "recreate");
  char hname[100];
  int Nbin = 16;
  float gmin = -350;
  float gmax = 350;
  int gNbin = 150;
  //int Nbin2=100;

  m_totalYX  = new TH2F("m_totalYX", " denominator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYX  = new TH2F("m_passYX", " numerator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_totalYZ  = new TH2F("m_totalYZ", " denominator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYZ  = new TH2F("m_passYZ", " numerator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX  = new TH2F("m_effiYX", " effi. Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYZ  = new TH2F("m_effiYZ", " effi. Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX->GetXaxis()->SetTitle("x (cm)");
  m_effiYX->GetYaxis()->SetTitle("y (cm)");
  m_effiYZ->GetXaxis()->SetTitle("z (cm)");
  m_effiYZ->GetYaxis()->SetTitle("y (cm)");
  //m_effiYX  = new TEfficiency("m_effiYX", "effi Y vs. X", 150, -350, 350, 150, -350, 350);
  //m_effiYZ  = new TEfficiency("m_effiYZ", "effi Y vs. Z", 150, -350, 350, 150, -350, 350);
  for (int iF = 0; iF < 2; iF++) {
    for (int iS = 0; iS < 8; iS++) {
      sprintf(hname, "effi_isForward%i_Sector%i", iF, iS + 1);
      m_effiVsLayer[iF][iS]  = new TEfficiency(hname, hname, Nbin, 0, 16);
      sprintf(hname, "total_isForward%i_Sector%i", iF, iS + 1);
      m_total[iF][iS] = new TH1F(hname, hname, Nbin, 0, 16);
      sprintf(hname, "pass_isForward%i_Sector%i", iF, iS + 1);
      m_pass[iF][iS] = new TH1F(hname, hname, Nbin, 0, 16);
    }
  }

}

void BKLMTrackingModule::beginRun()
{
}

void BKLMTrackingModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  //unsigned long eventNumber = eventMetaData->getEvent();
  //unsigned long runNumber = eventMetaData->getRun();
  //unsigned long expNumber = eventMetaData->getExperiment();

  //StoreArray<BKLMHit2d> hits2D;
  //StoreArray<BKLMTrack> m_storeTracks;
  m_storeTracks.clear();

  if (!m_studyEffi) {
    runTracking(0, -1, -1, -1);
  } else if (m_studyEffi) {
    for (int iForward = 0; iForward < 2; iForward++) {
      for (int iSector = 0; iSector < 8; iSector++) {
        //if(iSector!=2&&iSector!=6) continue;
        for (int iLayer = 0; iLayer < 15; iLayer++) {
          runTracking(1, iForward, iSector , iLayer);
          generateEffi(iForward, iSector, iLayer);
          //clear tracks so prepare for the next layer efficieny study
          m_storeTracks.clear();
        }
      }
    }

  }
}

void BKLMTrackingModule::runTracking(int mode, int iForward, int iSector, int iLayer)
{
  m_storeTracks.clear();
  //std::list<BKLMTrack*> tracks;
  //tracks.clear();

  //cout<<" mode "<<mode<<", iForward "<<iForward<<", "<<iSector<<" , "<<iLayer<<endl;
  BKLMTrackFitter* m_fitter = new BKLMTrackFitter();
  BKLMTrackFinder*  m_finder = new BKLMTrackFinder();
  m_finder->setGlobalFit(m_globalFit);
  if (mode == 1) m_finder->setGlobalFit(false);
  m_finder->registerFitter(m_fitter);

  if (hits2D.getEntries() < 1) return;
  if (mode == 1) { //efficieny study
    for (int j = 0; j < hits2D.getEntries(); j++) {
      hits2D[j]->isOnStaTrack(false);
    }
  }
  //cout<<" num. of 2D hits "<<hits2D.getEntries()<<endl;

  for (int hi = 0; hi < hits2D.getEntries() - 1; ++hi) {

    if (mode == 1 && isLayerUnderStudy(iForward, iSector, iLayer, hits2D[hi])) continue;
    if (mode == 1 && !isSectorUnderStudy(iForward, iSector, hits2D[hi])) continue;
    if (hits2D[hi]->isOnStaTrack()) continue;
    if (hits2D[hi]->isOutOfTime()) continue;
    for (int hj = hi + 1; hj < hits2D.getEntries(); ++hj) {

      if (hits2D[hj]->isOnStaTrack()) { continue; }
      if (hits2D[hj]->isOutOfTime()) continue;
      if (!m_globalFit && !sameSector(hits2D[hi], hits2D[hj])) { continue; }
      if (sameSector(hits2D[hi], hits2D[hj]) && abs(hits2D[hi]->getLayer() - hits2D[hj]->getLayer()) < 3) { continue;}

      std::list<BKLMHit2d*> sectorHitList;
      //sectorHitList.push_back(hits2D[hi]);
      //sectorHitList.push_back(hits2D[hj]);

      std::list<BKLMHit2d*> seed;
      seed.push_back(hits2D[hi]);
      seed.push_back(hits2D[hj]);

      for (int ho = 0; ho < hits2D.getEntries(); ++ho) {

        if (ho == hi || ho == hj) continue; //exclude seed hits
        if (mode == 1 && isLayerUnderStudy(iForward, iSector, iLayer, hits2D[hj])) continue;
        if (mode == 1 && !isSectorUnderStudy(iForward, iSector, hits2D[hj])) continue;
        if (hits2D[ho]->isOnStaTrack()) continue;
        if (!m_globalFit && !sameSector(hits2D[ho], hits2D[hi])) continue;
        // if (hits2D[ho]->getLayer() == hits2D[hi]->getLayer() || hits2D[ho]->getLayer() == hits2D[hj]->getLayer()) continue;
        if (hits2D[ho]->isOutOfTime()) continue;
        sectorHitList.push_back(hits2D[ho]);
      }

      /* Require at least four hits (minimum for good track, already two as seed, so here we require 2) but
       * no more than 60 (most likely noise, 60 would be four good tracks).
       */
      if (sectorHitList.size() < 2 || sectorHitList.size() > 60) continue;

      std::list<BKLMHit2d*> m_hits;
      if (m_finder->filter(seed, sectorHitList, m_hits)) {
        //B2INFO("BKLMTrackingModule::stand-alone BKLMTrack fitted success.");
        BKLMTrack* m_track = m_storeTracks.appendNew();
        m_track->setTrackParam(m_fitter->getTrackParam());
        m_track->setTrackParamErr(m_fitter->getTrackParamErr());
        m_track->setLocalTrackParam(m_fitter->getTrackParamSector());
        m_track->setLocalTrackParamErr(m_fitter->getTrackParamSectorErr());
        m_track->setTrackChi2(m_fitter->getChi2());
        m_track->setNumHitOnTrack(m_fitter->getNumHit());
        m_track->setIsValid(m_fitter->isValid());
        m_track->setIsGood(m_fitter->isGood());
        std::list<BKLMHit2d*>::iterator j;
        m_hits.sort(sortByLayer);
        for (j = m_hits.begin(); j != m_hits.end(); ++j) {
          (*j)->isOnStaTrack(true);
          m_track->addRelationTo((*j));
        }
        //tracks.push_back(m_track);
        //m_track->getTrackParam().Print();
        //m_track->getTrackParamErr().Print();
        //match BKLMTrack to RecoTrack
        if (mode == 0) {
          RecoTrack* closestTrack = nullptr;
          if (m_MatchToRecoTrack) {
            if (findClosestRecoTrack(m_track, closestTrack)) {
              //B2INFO("BKLMTrackingModule::matched RecoTrack found.");
              m_track->addRelationTo(closestTrack);
              for (j = m_hits.begin(); j != m_hits.end(); ++j) {
                unsigned int sortingParameter = closestTrack->getNumberOfTotalHits();
                closestTrack->addBKLMHit((*j), sortingParameter, RecoHitInformation::OriginTrackFinder::c_LocalTrackFinder);
              }
            }
          }//end match
        }
      }
    }
  }

  delete m_fitter;
  delete m_finder;

}

void BKLMTrackingModule::endRun()
{
}

void BKLMTrackingModule::terminate()
{


  m_file->cd();
  char name[100];
  for (int iF = 0; iF < 2; iF++) {
    for (int iS = 0; iS < 8; iS++) {
      sprintf(name, "isForward%i_Sector%i_check", iF, iS + 1);
      //m_effiVsLayer_check[iF][iS]  = new TEfficiency(*m_pass[iF][iS], *m_total[iF][iS]);
      //m_effiVsLayer_check[iF][iS]->SetName(name);
      m_effiVsLayer[iF][iS]->Write();
      m_total[iF][iS]->Write();
      m_pass[iF][iS]->Write();
      //m_theta[iF][iS]->Write();
      //m_phi[iF][iS]->Write();
      //m_effiVsLayer_check[iF][iS]->Write();
      /*for(int iL = 0; iL<15; iL++)
      {
      m_effiVsPhi[iF][iS][iL]->Write();
      }
      */
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

  m_totalYX->Write();
  m_passYX->Write();
  m_totalYZ->Write();
  m_passYZ->Write();
  m_effiYX->Write();
  m_effiYZ->Write();
  m_file->Close();

}

bool BKLMTrackingModule::sameSector(BKLMHit2d* hit1, BKLMHit2d* hit2)
{
  if (hit1->isForward() == hit2->isForward() && hit1->getSector() == hit2->getSector()) return true;
  else return false;
}


bool BKLMTrackingModule::findClosestRecoTrack(BKLMTrack* bklmTrk, RecoTrack*& closestTrack)
{

  //StoreArray<RecoTrack> recoTracks;
  RelationVector<BKLMHit2d> bklmHits = bklmTrk->getRelationsTo<BKLMHit2d> ();

  if (bklmHits.size() < 1) { B2INFO("BKLMTrackingModule::something is wrong! there is BKLMTrack but no bklmHits"); return false;}
  if (recoTracks.getEntries() < 1) { B2INFO("BKLMTrackingModule::there is no recoTrack"); return false;}
  double oldDistance = INFINITY;
  double oldAngle = INFINITY;
  closestTrack = nullptr;
  //TVector3 poca = TVector3(0, 0, 0);
  TVector3 firstBKLMHitPosition(0, 0, 0);
  //bklmHits are already sorted by layer
  //possible two hits in one layer?
  firstBKLMHitPosition = bklmHits[0]->getGlobalPosition();

  TMatrixDSym cov(6);
  TVector3 pos(0, 0, 0);
  TVector3 mom(0, 0, 0);

  for (RecoTrack& track : recoTracks) {
    try {
      genfit::MeasuredStateOnPlane state = track.getMeasuredStateOnPlaneFromLastHit();
      //! Translates MeasuredStateOnPlane into 3D position, momentum and 6x6 covariance.
      state.getPosMomCov(pos, mom, cov);
      if (mom.Y() * pos.Y() < 0)
      { state = track.getMeasuredStateOnPlaneFromFirstHit(); }
      //pos.Print(); mom.Print();
      const TVector3& distanceVec = firstBKLMHitPosition - pos;
      state.extrapolateToPoint(firstBKLMHitPosition);
      double newDistance = distanceVec.Mag2();
      // two points on the track, (x1,TrkParam[0]+TrkParam[1]*x1, TrkParam[2]+TrkParam[3]*x1),
      // and (x2,TrkParam[0]+TrkParam[1]*x2, TrkParam[2]+TrkParam[3]*x2),
      // then we got the vector (x2-x1,....), that is same with (1,TrkParam[1], TrkParam[3]).
      TVector3 trkVec(1, bklmTrk->getTrackParam()[1], bklmTrk->getTrackParam()[3]);
      double angle = trkVec.Angle(mom);
      // choose closest distance or minimum open angle ?
      // overwrite old distance
      if (newDistance < oldDistance) {
        oldDistance = newDistance;
        closestTrack = &track;
        //poca = pos;
        oldAngle = angle;
      }

      /* if(angle<oldAngle)
      {
      oldAngle=angle;
      closestTrack = &track;
      }
      */
    } catch (genfit::Exception& e) {
    }// try
  }

  // can not find matched RecoTrack
  // problem here is the errors of the track parameters are not considered!
  // best way is the positon or vector direction are required within 5/10 sigma ?
  if (oldAngle > m_maxAngleRequired) return false;
  // found matched RecoTrack
  else return true;
}

void BKLMTrackingModule::generateEffi(int iForward, int iSector, int iLayer)
{

  set<int> m_pointUsed;
  m_pointUsed.clear();
  if (m_storeTracks.getEntries() < 1) return;

  for (int it = 0; it < m_storeTracks.getEntries(); it++) {
    //if(m_storeTracks[it]->getTrackChi2()>10) continue;
    //if(m_storeTracks[it]->getNumHitOnTrack()<6) continue;
    int cnt1 = 0;
    int cnt2 = 0;

    RelationVector<BKLMHit2d> relatedHit2D = m_storeTracks[it]->getRelationsTo<BKLMHit2d>();
    for (const BKLMHit2d& hit2D : relatedHit2D) {
      if (hit2D.getLayer() > iLayer + 1) cnt1 ++;
      if (hit2D.getLayer() < iLayer + 1) cnt2 ++;
    }

    if (iLayer != 0 && cnt2 < 1) return;
    if (iLayer != 14 && cnt1 < 1) return;
    m_GeoPar = GeometryPar::instance();
    const bklm::Module* module = m_GeoPar->findModule(iForward, iSector + 1, iLayer + 1);
    int minPhiStrip = module->getPhiStripMin();
    int maxPhiStrip = module->getPhiStripMax();
    int minZStrip = module->getZStripMin();
    int maxZStrip = module->getZStripMax();

    CLHEP::Hep3Vector local = module->getLocalPosition(minPhiStrip, minZStrip);
    CLHEP::Hep3Vector local2 = module->getLocalPosition(maxPhiStrip, maxZStrip);
    float minLocalY, maxLocalY;
    float minLocalZ, maxLocalZ;
    if (local[1] > local2[1]) { maxLocalY = local[1]; minLocalY = local2[1]; } else { maxLocalY = local2[1]; minLocalY = local[1];}
    if (local[2] > local2[2]) { maxLocalZ = local[2]; minLocalZ = local2[2]; } else { maxLocalZ = local2[2]; minLocalZ = local[2];}

    TVectorD trkPar = m_storeTracks[it]->getLocalTrackParam();

    //first layer is the reference layer
    //if (iForward == 1 && (iSector + 1 ) ==5)  cout<<" local X "<<m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1, iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1, 1)<<endl;
    float reflocalX = fabs(m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1,
                                                           iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1, 1));
    //if (iForward == 1 && (iSector + 1 ) ==5)  cout<<" local X "<<m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1, iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iForward, iSector + 1, 1)<<endl;

    float reflocalY = trkPar[0] + trkPar[1] * reflocalX;
    float reflocalZ = trkPar[2] + trkPar[3] * reflocalX;

    //reference module is the first layer
    //module = m_GeoPar->findModule(iForward, iSector + 1, 1);
    reflocalX = 0.0;
    Hep3Vector reflocal(reflocalX, reflocalY, reflocalZ);
    //Hep3Vector global(localX, localY, localZ);
    Hep3Vector global(0, 0, 0);
    module = m_GeoPar->findModule(iForward, iSector + 1, iLayer + 1);
    global = module->localToGlobal(reflocal);
    //float localX = module->globalToLocal(global)[0];
    float localY = module->globalToLocal(global)[1];
    float localZ = module->globalToLocal(global)[2];


    //geometry cut
    if (localY > minLocalY && localY < maxLocalY && localZ > minLocalZ && localZ < maxLocalZ) {

      bool m_iffound = false;
      m_total[iForward][iSector]->Fill(iLayer + 1);
      //cout<<" isForwadrd "<<iForward<<", layer "<<iLayer + 1<<endl;
      m_totalYX->Fill(global[0], global[1]);
      m_totalYZ->Fill(global[2], global[1]);

      for (int he = 0; he < hits2D.getEntries(); ++he) {
        if (!isLayerUnderStudy(iForward, iSector, iLayer, hits2D[he])) continue;
        if (hits2D[he]->isOutOfTime()) continue;
        //if alreday used, skip
        if (m_pointUsed.find(he) != m_pointUsed.end()) continue;

        double error, sigma;
        float distance = distanceToHit(m_storeTracks[it], hits2D[he], error, sigma);

        if (distance < 10 && sigma < 5) { m_iffound = true; }
        if (m_iffound) {
          m_pointUsed.insert(he);
          //global[0] = hits2D[he]->getGlobalPosition()[0];
          //global[1] = hits2D[he]->getGlobalPosition()[1];
          //global[2] = hits2D[he]->getGlobalPosition()[2];
          m_pass[iForward][iSector]->Fill(iLayer + 1);
          m_passYX->Fill(global[0], global[1]);
          m_passYZ->Fill(global[2], global[1]);
          break;
        }
      }

      m_effiVsLayer[iForward][iSector]->Fill(m_iffound, iLayer + 1);
      //cout<<" global "<<global[0]<<", "<< global[1]<<" "<<global[2]<<endl;
      //m_effiYX->Fill(m_iffound, global[1], global[0]);
      //m_effiYZ->Fill(m_iffound, global[1], global[2]);
      //m_effiYX->SetPassedHistogram(*m_passYX);
      //m_effiYX->SetTotalHistogram(*m_totalYX);
      //m_effiYZ->SetPassedHistogram(*m_passYZ);
      //m_effiYZ->SetTotalHistogram(*m_totalYZ);
    }
  }//end of loop tracks

}

bool BKLMTrackingModule::sortByLayer(BKLMHit2d* hit1, BKLMHit2d* hit2)
{

  return hit1->getLayer() < hit2->getLayer();

}

bool BKLMTrackingModule::isLayerUnderStudy(int isForward, int iSector, int iLayer, BKLMHit2d* hit)
{
  if (hit->isForward() == isForward && hit->getSector() == iSector + 1 &&  hit->getLayer() == iLayer + 1) return true;
  else return false;
}

bool BKLMTrackingModule::isSectorUnderStudy(int isForward, int iSector, BKLMHit2d* hit)
{
  if (hit->isForward() == isForward && hit->getSector() == iSector + 1) return true;
  else return false;
}

double BKLMTrackingModule::distanceToHit(BKLMTrack* track, BKLMHit2d* hit,
                                         double& error,
                                         double& sigma)
{

  double x, y, z, dy, dz;

  error = DBL_MAX;
  sigma = DBL_MAX;

  TVectorD m_SectorPar = track->getLocalTrackParam();
  TMatrixDSym m_SectorErr = track->getTrackParamErr();

  m_GeoPar = GeometryPar::instance();
  const Belle2::bklm::Module* refMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), 1);
  const Belle2::bklm::Module* corMod = m_GeoPar->findModule(hit->isForward(), hit->getSector(), hit->getLayer());

  CLHEP::Hep3Vector globalPos(hit->getGlobalPosition()[0], hit->getGlobalPosition()[1], hit->getGlobalPosition()[2]);
  CLHEP::Hep3Vector local = refMod->globalToLocal(globalPos);

  x = local[0] ;

  y = m_SectorPar[ 0 ] + x * m_SectorPar[ 1 ];
  z = m_SectorPar[ 2 ] + x * m_SectorPar[ 3 ];

  dy = y - local[1];
  dz = z - local[2];

  double distance = sqrt(dy * dy + dz * dz);

  double hit_localPhiErr = corMod->getPhiStripWidth() / sqrt(12);
  double hit_localZErr = corMod->getZStripWidth() / sqrt(12);

  //error from tracking is ignored here
  error = sqrt(pow(hit_localPhiErr, 2) +
               pow(hit_localZErr, 2));

  if (error != 0.0) {
    sigma = distance / error;
  } else {
    sigma = DBL_MAX;
  }

  return (distance);

}
