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
#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;

REG_MODULE(BKLMTracking)

BKLMTrackingModule::BKLMTrackingModule() : Module()
{
  setDescription("perform standard-alone straight line tracking for BKLM");
  addParam("MeanDt", m_MeanDt, "[ns] Mean hit-trigger time for coincident hits (default 0)", double(0.0));
  addParam("MaxDt", m_MaxDt, "[ns] Coincidence window half-width for in-time KLM hits (default +-30)", double(30.0));
  addParam("MatchToRecoTrack", m_MatchToRecoTrack, "[bool], whether match BKLMTrack to RecoTrack; (default is false)", false);
  addParam("MaxAngleRequired", m_maxAngleRequired,
           "[degree], match BKLMTrack to RecoTrack; angle between them is required to be smaller than (default 10)", double(10.0));
}

BKLMTrackingModule::~BKLMTrackingModule()
{

}

void BKLMTrackingModule::initialize()
{

  StoreArray<BKLMTrack> m_storeTracks;
  StoreArray<BKLMHit2d> hits2D;
  StoreArray<RecoTrack> recoTracks;
  StoreArray<RecoHitInformation> recoHitInformation;
  m_storeTracks.registerInDataStore();
  m_storeTracks.registerRelationTo(hits2D);
  m_storeTracks.registerRelationTo(recoTracks);
  recoHitInformation.registerRelationTo(hits2D);
  hits2D.registerRelationTo(recoTracks);

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

  StoreArray<BKLMHit2d> hits2D;
  StoreArray<BKLMTrack> m_storeTracks;


  std::list<BKLMTrack*> tracks;
  BKLMTrackFitter* m_fitter = new BKLMTrackFitter();
  BKLMTrackFinder*  m_finder = new BKLMTrackFinder();
  m_finder->registerFitter(m_fitter);

  //if (hits2D.getEntries() < 1) return;

  for (int hi = 0; hi < hits2D.getEntries() - 1; ++hi) {

    if (hits2D[hi]->isOnStaTrack()) continue;
    if ((hits2D[hi]->getTime() - m_MeanDt) > m_MaxDt) continue;
    for (int hj = hi + 1; hj < hits2D.getEntries(); ++hj) {

      if (hits2D[hj]->isOnStaTrack()) { continue; }
      if ((hits2D[hj]->getTime() - m_MeanDt) > m_MaxDt) continue;
      if (!sameSector(hits2D[hi], hits2D[hj])) { continue; }
      if (abs(hits2D[hi]->getLayer() - hits2D[hj]->getLayer()) < 3) { continue;}

      std::list<BKLMHit2d*> sectorHitList;
      //sectorHitList.push_back(hits2D[hi]);
      //sectorHitList.push_back(hits2D[hj]);

      std::list<BKLMHit2d*> seed;
      seed.push_back(hits2D[hi]);
      seed.push_back(hits2D[hj]);

      for (int ho = 0; ho < hits2D.getEntries(); ++ho) {

        if (ho == hi || ho == hj) continue; //exclude seed hits
        if (hits2D[ho]->isOnStaTrack()) continue;
        if (!sameSector(hits2D[ho], hits2D[hi])) continue;
        // if (hits2D[ho]->getLayer() == hits2D[hi]->getLayer() || hits2D[ho]->getLayer() == hits2D[hj]->getLayer()) continue;
        if ((hits2D[ho]->getTime() - m_MeanDt) > m_MaxDt) continue;
        sectorHitList.push_back(hits2D[ho]);
      }

      /* Require at least four hits (minimum for good track, already two as seed, so here we require 2) but
       * no more than 60 (most likely noise, 60 would be four good tracks).
       */
      if (sectorHitList.size() < 2 || sectorHitList.size() > 60) continue;

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
        tracks.push_back(m_track);
        //m_track->getTrackParam().Print();
        //m_track->getTrackParamErr().Print();
        //match BKLMTrack to RecoTrack
        RecoTrack* closestTrack = nullptr;
        if (m_MatchToRecoTrack) {
          if (findClosestRecoTrack(m_track, closestTrack)) {
            m_track->addRelationTo(closestTrack);
            for (j = m_hits.begin(); j != m_hits.end(); ++j) {
              unsigned int sortingParameter = closestTrack->getNumberOfTotalHits();
              closestTrack->addBKLMHit((*j), sortingParameter, RecoHitInformation::OriginTrackFinder::c_LocalTrackFinder);
            }
          }
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
}

bool BKLMTrackingModule::sameSector(BKLMHit2d* hit1, BKLMHit2d* hit2)
{
  if (hit1->isForward() == hit2->isForward() && hit1->getSector() == hit2->getSector()) return true;
  else return false;
}


bool BKLMTrackingModule::findClosestRecoTrack(BKLMTrack* bklmTrk, RecoTrack*& closestTrack)
{

  StoreArray<RecoTrack> recoTracks;
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
      state.extrapolateToPoint(firstBKLMHitPosition);
      //! Translates MeasuredStateOnPlane into 3D position, momentum and 6x6 covariance.
      state.getPosMomCov(pos, mom, cov);
      //pos.Print(); mom.Print();
      const TVector3& distanceVec = firstBKLMHitPosition - pos;
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

bool BKLMTrackingModule::sortByLayer(BKLMHit2d* hit1, BKLMHit2d* hit2)
{

  return hit1->getLayer() < hit2->getLayer();

}
