/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/bklmTracking/BKLMTrackingModule.h>

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/bklm/modules/bklmTracking/BKLMTrackFinder.h>

/* Belle 2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;
using namespace CLHEP;

REG_MODULE(BKLMTracking)

BKLMTrackingModule::BKLMTrackingModule() : Module(),
  m_effiYX(nullptr),
  m_effiYZ(nullptr),
  m_passYX(nullptr),
  m_totalYX(nullptr),
  m_passYZ(nullptr),
  m_totalYZ(nullptr),
  m_runTotalEvents(0),
  m_runTotalEventsWithTracks(0)
{
  for (int i = 0; i < 8; ++i) {
    m_total[0][i] = nullptr;
    m_total[1][i] = nullptr;
    m_pass[0][i] = nullptr;
    m_pass[1][i] = nullptr;
    m_effiVsLayer[0][i] = nullptr;
    m_effiVsLayer[1][i] = nullptr;
  }
  setDescription("Perform standard-alone straight line tracking for BKLM");
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

  if (m_studyEffi)
    B2INFO("BKLMTrackingModule:: this module is running in efficiency study mode!");

  m_file =     new TFile(m_outPath.c_str(), "recreate");
  TString hname;
  std::string labelFB[2] = {"BB", "BF"};
  int Nbin = 16;
  float gmin = -350;
  float gmax = 350;
  int gNbin = 150;

  m_totalYX  = new TH2F("totalYX", " denominator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYX  = new TH2F("passYX", " numerator Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_totalYZ  = new TH2F("totalYZ", " denominator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_passYZ  = new TH2F("passYZ", " numerator Y vs. Z", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX  = new TH2F("effiYX", " effi. Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYZ  = new TH2F("effiYZ", " effi. Y vs. X", gNbin, gmin, gmax, gNbin, gmin, gmax);
  m_effiYX->GetXaxis()->SetTitle("x (cm)");
  m_effiYX->GetYaxis()->SetTitle("y (cm)");
  m_effiYZ->GetXaxis()->SetTitle("z (cm)");
  m_effiYZ->GetYaxis()->SetTitle("y (cm)");
  for (int iF = 0; iF < 2; iF++) {
    for (int iS = 0; iS < 8; iS++) {
      hname.Form("effi_%s%i", labelFB[iF].c_str(), iS);
      m_effiVsLayer[iF][iS]  = new TEfficiency(hname, hname, Nbin, 0, 16);
      hname.Form("total_%s%i", labelFB[iF].c_str(), iS);
      m_total[iF][iS] = new TH1F(hname, hname, Nbin, 0, 16);
      hname.Form("pass_%s%i", labelFB[iF].c_str(), iS);
      m_pass[iF][iS] = new TH1F(hname, hname, Nbin, 0, 16);
    }
  }

}

void BKLMTrackingModule::beginRun()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_runNumber.push_back((int)eventMetaData->getRun());
  m_runTotalEvents = 0;
  m_runTotalEventsWithTracks = 0;
}

void BKLMTrackingModule::event()
{
  m_storeTracks.clear();
  bool thereIsATrack = false;

  if (!m_studyEffi) {
    runTracking(0, -1, -1, -1);
    if (m_storeTracks.getEntries() > 0)
      thereIsATrack = true;
  } else {
    for (int iSection = 0; iSection < 2; iSection++) {
      for (int iSector = 0; iSector < 8; iSector++) {
        for (int iLayer = 0; iLayer < 15; iLayer++) {
          runTracking(1, iSection, iSector , iLayer);
          if (m_storeTracks.getEntries() > 0)
            thereIsATrack = true;
          generateEffi(iSection, iSector, iLayer);
          //clear tracks so prepare for the next layer efficieny study
          m_storeTracks.clear();
        }
      }
    }
  }

  m_runTotalEvents++;
  if (thereIsATrack)
    m_runTotalEventsWithTracks++;
}

void BKLMTrackingModule::runTracking(int mode, int iSection, int iSector, int iLayer)
{
  m_storeTracks.clear();
  //std::list<BKLMTrack*> tracks;
  //tracks.clear();

  BKLMTrackFitter* m_fitter = new BKLMTrackFitter();
  BKLMTrackFinder*  m_finder = new BKLMTrackFinder();
  m_finder->setGlobalFit(m_globalFit);
  if (mode == 1)
    m_finder->setGlobalFit(false);
  m_finder->registerFitter(m_fitter);

  if (hits2D.getEntries() < 1)
    return;
  if (mode == 1) { //efficieny study
    for (int j = 0; j < hits2D.getEntries(); j++) {
      hits2D[j]->isOnStaTrack(false);
    }
  }

  for (int hi = 0; hi < hits2D.getEntries() - 1; ++hi) {

    if (mode == 1 && isLayerUnderStudy(iSection, iSector, iLayer, hits2D[hi]))
      continue;
    if (mode == 1 && !isSectorUnderStudy(iSection, iSector, hits2D[hi]))
      continue;
    if (hits2D[hi]->isOnStaTrack())
      continue;
    if (hits2D[hi]->isOutOfTime())
      continue;
    for (int hj = hi + 1; hj < hits2D.getEntries(); ++hj) {

      if (hits2D[hj]->isOnStaTrack())
        continue;
      if (hits2D[hj]->isOutOfTime())
        continue;
      if (!m_globalFit && !sameSector(hits2D[hi], hits2D[hj]))
        continue;
      if (sameSector(hits2D[hi], hits2D[hj]) && abs(hits2D[hi]->getLayer() - hits2D[hj]->getLayer()) < 3)
        continue;

      std::list<BKLMHit2d*> sectorHitList;
      //sectorHitList.push_back(hits2D[hi]);
      //sectorHitList.push_back(hits2D[hj]);

      std::list<BKLMHit2d*> seed;
      seed.push_back(hits2D[hi]);
      seed.push_back(hits2D[hj]);

      for (int ho = 0; ho < hits2D.getEntries(); ++ho) {

        // Exclude seed hits.
        if (ho == hi || ho == hj)
          continue;
        if (mode == 1 && isLayerUnderStudy(iSection, iSector, iLayer, hits2D[hj]))
          continue;
        if (mode == 1 && !isSectorUnderStudy(iSection, iSector, hits2D[hj]))
          continue;
        if (hits2D[ho]->isOnStaTrack())
          continue;
        if (!m_globalFit && !sameSector(hits2D[ho], hits2D[hi]))
          continue;
        // if (hits2D[ho]->getLayer() == hits2D[hi]->getLayer() || hits2D[ho]->getLayer() == hits2D[hj]->getLayer())
        //   continue;
        if (hits2D[ho]->isOutOfTime())
          continue;
        sectorHitList.push_back(hits2D[ho]);
      }

      /* Require at least four hits (minimum for good track, already two as seed, so here we require 2) but
       * no more than 60 (most likely noise, 60 would be four good tracks).
       */
      if (sectorHitList.size() < 2 || sectorHitList.size() > 60)
        continue;

      std::list<BKLMHit2d*> m_hits;
      if (m_finder->filter(seed, sectorHitList, m_hits)) {
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
  m_totalEvents.push_back(m_runTotalEvents);
  m_totalEventsWithTracks.push_back(m_runTotalEventsWithTracks);
}

void BKLMTrackingModule::terminate()
{
  for (long unsigned int i = 0; i < m_runNumber.size(); i++) {
    float ratio = (float)m_totalEventsWithTracks.at(i) / (float)m_totalEvents.at(i);
    B2INFO("BKLMTrackingModule:: run " << m_runNumber.at(i) << " --> " << ratio * 100 << "% of events has 1+ BKLMTracks");
  }

  m_file->cd();
  for (int iF = 0; iF < 2; iF++) {
    for (int iS = 0; iS < 8; iS++) {
      m_effiVsLayer[iF][iS]->Write();
      m_total[iF][iS]->Write();
      m_pass[iF][iS]->Write();
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
  if (hit1->getSection() == hit2->getSection() && hit1->getSector() == hit2->getSector())
    return true;
  else return false;
}


bool BKLMTrackingModule::findClosestRecoTrack(BKLMTrack* bklmTrk, RecoTrack*& closestTrack)
{

  //StoreArray<RecoTrack> recoTracks;
  RelationVector<BKLMHit2d> bklmHits = bklmTrk->getRelationsTo<BKLMHit2d> ();

  if (bklmHits.size() < 1) {
    B2INFO("BKLMTrackingModule::something is wrong! there is BKLMTrack but no bklmHits");
    return false;
  }
  if (recoTracks.getEntries() < 1) {
    B2INFO("BKLMTrackingModule::there is no recoTrack");
    return false;
  }
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
  if (oldAngle > m_maxAngleRequired)
    return false;
  // found matched RecoTrack
  else return true;
}

void BKLMTrackingModule::generateEffi(int iSection, int iSector, int iLayer)
{

  set<int> m_pointUsed;
  m_pointUsed.clear();
  if (m_storeTracks.getEntries() < 1)
    return;

  for (int it = 0; it < m_storeTracks.getEntries(); it++) {
    //if(m_storeTracks[it]->getTrackChi2()>10) continue;
    //if(m_storeTracks[it]->getNumHitOnTrack()<6) continue;
    int cnt1 = 0;
    int cnt2 = 0;

    RelationVector<BKLMHit2d> relatedHit2D = m_storeTracks[it]->getRelationsTo<BKLMHit2d>();
    for (const BKLMHit2d& hit2D : relatedHit2D) {
      if (hit2D.getLayer() > iLayer + 1)
        cnt1++;
      if (hit2D.getLayer() < iLayer + 1)
        cnt2++;
    }

    if (iLayer != 0 && cnt2 < 1)
      return;
    if (iLayer != 14 && cnt1 < 1)
      return;
    m_GeoPar = GeometryPar::instance();
    const bklm::Module* module = m_GeoPar->findModule(iSection, iSector + 1, iLayer + 1);
    int minPhiStrip = module->getPhiStripMin();
    int maxPhiStrip = module->getPhiStripMax();
    int minZStrip = module->getZStripMin();
    int maxZStrip = module->getZStripMax();

    CLHEP::Hep3Vector local = module->getLocalPosition(minPhiStrip, minZStrip);
    CLHEP::Hep3Vector local2 = module->getLocalPosition(maxPhiStrip, maxZStrip);
    float minLocalY, maxLocalY;
    float minLocalZ, maxLocalZ;
    if (local[1] > local2[1]) {
      maxLocalY = local[1];
      minLocalY = local2[1];
    } else {
      maxLocalY = local2[1];
      minLocalY = local[1];
    }
    if (local[2] > local2[2]) {
      maxLocalZ = local[2];
      minLocalZ = local2[2];
    } else {
      maxLocalZ = local2[2];
      minLocalZ = local[2];
    }

    TVectorD trkPar = m_storeTracks[it]->getLocalTrackParam();

    //first layer is the reference layer
    //if (iSection == 1 && (iSector + 1 ) == 5)
    //  cout<<" local X "<<m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1, iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1, 1) << endl;
    float reflocalX = fabs(m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1,
                                                           iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1, 1));
    //if (iSection == 1 && (iSector + 1 ) == 5)
    //  cout<<" local X "<<m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1, iLayer + 1) - m_GeoPar->getActiveMiddleRadius(iSection, iSector + 1, 1) << endl;

    float reflocalY = trkPar[0] + trkPar[1] * reflocalX;
    float reflocalZ = trkPar[2] + trkPar[3] * reflocalX;

    //reference module is the first layer
    //module = m_GeoPar->findModule(iSection, iSector + 1, 1);
    reflocalX = 0.0;
    Hep3Vector reflocal(reflocalX, reflocalY, reflocalZ);
    //Hep3Vector global(localX, localY, localZ);
    Hep3Vector global(0, 0, 0);
    module = m_GeoPar->findModule(iSection, iSector + 1, iLayer + 1);
    global = module->localToGlobal(reflocal);
    //float localX = module->globalToLocal(global)[0];
    float localY = module->globalToLocal(global)[1];
    float localZ = module->globalToLocal(global)[2];


    //geometry cut
    if (localY > minLocalY && localY < maxLocalY && localZ > minLocalZ && localZ < maxLocalZ) {

      bool m_iffound = false;
      m_total[iSection][iSector]->Fill(iLayer + 1);
      m_totalYX->Fill(global[0], global[1]);
      m_totalYZ->Fill(global[2], global[1]);

      for (int he = 0; he < hits2D.getEntries(); ++he) {
        if (!isLayerUnderStudy(iSection, iSector, iLayer, hits2D[he]))
          continue;
        if (hits2D[he]->isOutOfTime())
          continue;
        //if alreday used, skip
        if (m_pointUsed.find(he) != m_pointUsed.end())
          continue;

        double error, sigma;
        float distance = distanceToHit(m_storeTracks[it], hits2D[he], error, sigma);

        if (distance < 10 && sigma < 5)
          m_iffound = true;
        if (m_iffound) {
          m_pointUsed.insert(he);
          //global[0] = hits2D[he]->getGlobalPosition()[0];
          //global[1] = hits2D[he]->getGlobalPosition()[1];
          //global[2] = hits2D[he]->getGlobalPosition()[2];
          m_pass[iSection][iSector]->Fill(iLayer + 1);
          m_passYX->Fill(global[0], global[1]);
          m_passYZ->Fill(global[2], global[1]);
          break;
        }
      }

      m_effiVsLayer[iSection][iSector]->Fill(m_iffound, iLayer + 1);
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

bool BKLMTrackingModule::isLayerUnderStudy(int section, int iSector, int iLayer, BKLMHit2d* hit)
{
  if (hit->getSection() == section && hit->getSector() == iSector + 1 &&  hit->getLayer() == iLayer + 1)
    return true;
  else return false;
}

bool BKLMTrackingModule::isSectorUnderStudy(int section, int iSector, BKLMHit2d* hit)
{
  if (hit->getSection() == section && hit->getSector() == iSector + 1)
    return true;
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

  m_GeoPar = GeometryPar::instance();
  const Belle2::bklm::Module* refMod = m_GeoPar->findModule(hit->getSection(), hit->getSector(), 1);
  const Belle2::bklm::Module* corMod = m_GeoPar->findModule(hit->getSection(), hit->getSector(), hit->getLayer());

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
