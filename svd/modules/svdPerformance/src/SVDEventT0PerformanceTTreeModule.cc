/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDEventT0PerformanceTTreeModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TDirectory.h>
#include <Math/Boost.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>

// Unpackers
#include <trg/ecl/dataobjects/TRGECLUnpackerStore.h>

// OnlineEventT0
#include <hlt/dataobjects/OnlineEventT0.h>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventT0PerformanceTTree);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventT0PerformanceTTreeModule::SVDEventT0PerformanceTTreeModule() : Module()
{
  //Set module properties
  setDescription("This module is used to create a TTree to study SVD eventT0 using clusters associated to tracks.");
  //Parameter to take only specific RecoTracks as input
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDEventT0PerformanceTTree.root"));
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "StoreArray name of the input and output RecoTracks.",
           m_recoTracksStoreArrayName);
}

void SVDEventT0PerformanceTTreeModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  recoTracks.isOptional();
  m_clusters.isRequired();
  m_EventT0.isOptional();

  TDirectory::TContext context;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //Tree for SVD clusters
  //one fill per event!
  m_t = new TTree("tree", "Tree for SVD clusters related to tracks");
  m_t->Branch("exp", &m_exp, "exp/i");
  m_t->Branch("run", &m_run, "run/i");
  m_t->Branch("event", &m_event, "event/i");
  m_t->Branch("trueEventT0", &m_trueEventT0, "trueEventT0/F");
  m_t->Branch("eventT0", &m_eventT0, "eventT0/F");
  m_t->Branch("eventT0Err", &m_eventT0Err, "eventT0Err/F");
  m_t->Branch("svdEventT0", &m_svdEventT0, "svdEventT0/F");
  m_t->Branch("svdEventT0Err", &m_svdEventT0Err, "svdEventT0Err/F");
  m_t->Branch("svdOnlineEventT0", &m_svdOnlineEventT0, "svdOnlineEventT0/F");
  m_t->Branch("svdOnlineEventT0Err", &m_svdOnlineEventT0Err, "svdOnlineEventT0Err/F");
  m_t->Branch("cdcEventT0", &m_cdcEventT0, "cdcEventT0/F");
  m_t->Branch("cdcEventT0Err", &m_cdcEventT0Err, "cdcEventT0Err/F");
  m_t->Branch("cdcOnlineEventT0", &m_cdcOnlineEventT0, "cdcOnlineEventT0/F");
  m_t->Branch("cdcOnlineEventT0Err", &m_cdcOnlineEventT0Err, "cdcOnlineEventT0Err/F");
  m_t->Branch("topEventT0", &m_topEventT0, "topEventT0/F");
  m_t->Branch("topEventT0Err", &m_topEventT0Err, "topEventT0Err/F");
  m_t->Branch("topOnlineEventT0", &m_topOnlineEventT0, "topOnlineEventT0/F");
  m_t->Branch("topOnlineEventT0Err", &m_topOnlineEventT0Err, "topOnlineEventT0Err/F");
  m_t->Branch("eclEventT0", &m_eclEventT0, "eclEventT0/F");
  m_t->Branch("eclEventT0Err", &m_eclEventT0Err, "eclEventT0Err/F");
  m_t->Branch("eclOnlineEventT0", &m_eclOnlineEventT0, "eclOnlineEventT0/F");
  m_t->Branch("eclOnlineEventT0Err", &m_eclOnlineEventT0Err, "eclOnlineEventT0Err/F");
  m_t->Branch("eclTCEmax", &m_eclTCEmax, "eclTCEmax/I");
  m_t->Branch("eclTCid", &m_eclTCid, "eclTCid/I");
  m_t->Branch("totTracks", &m_nTracks, "totTracks/i");
  m_t->Branch("TB", &m_svdTB, "TB/i");
  m_t->Branch("trkNumb", &m_trkNumber);
  m_t->Branch("trkd0", &m_svdTrkd0);
  m_t->Branch("trkz0", &m_svdTrkz0);
  m_t->Branch("trkp", &m_svdTrkp);
  m_t->Branch("trkpT", &m_svdTrkpT);
  m_t->Branch("trkpCM", &m_svdTrkpCM);
  m_t->Branch("trkTheta", &m_svdTrkTheta);
  m_t->Branch("trkPhi", &m_svdTrkPhi);
  m_t->Branch("trkCharge", &m_svdTrkCharge);
  m_t->Branch("trkPValue", &m_svdTrkPValue);
  m_t->Branch("trkNDF", &m_svdTrkNDF);
  m_t->Branch("trkPXDHits", &m_svdTrkPXDHits);
  m_t->Branch("trkSVDHits", &m_svdTrkSVDHits);
  m_t->Branch("trkCDCHits", &m_svdTrkCDCHits);
  m_t->Branch("layer", &m_svdLayer);
  m_t->Branch("ladder", &m_svdLadder);
  m_t->Branch("sensor", &m_svdSensor);
  m_t->Branch("clsSize", &m_svdSize);
  m_t->Branch("clsIsU", &m_svdisUside);
  m_t->Branch("clsCharge", &m_svdClCharge);
  m_t->Branch("clsSNR", &m_svdClSNR);
  m_t->Branch("clsPos", &m_svdClPos);
  m_t->Branch("clsPosErr", &m_svdClPosErr);
  m_t->Branch("clsTime", &m_svdClTime);
  m_t->Branch("clsTimeErr", &m_svdClTimeErr);
  m_t->Branch("trueTime", &m_svdTrueTime);



}

void SVDEventT0PerformanceTTreeModule::event()
{
  m_trueEventT0         = std::numeric_limits<float>::quiet_NaN();
  m_eventT0             = std::numeric_limits<float>::quiet_NaN();
  m_eventT0Err          = std::numeric_limits<float>::quiet_NaN();
  m_svdEventT0          = std::numeric_limits<float>::quiet_NaN();
  m_svdEventT0Err       = std::numeric_limits<float>::quiet_NaN();
  m_svdOnlineEventT0    = std::numeric_limits<float>::quiet_NaN();
  m_svdOnlineEventT0Err = std::numeric_limits<float>::quiet_NaN();
  m_cdcEventT0          = std::numeric_limits<float>::quiet_NaN();
  m_cdcEventT0Err       = std::numeric_limits<float>::quiet_NaN();
  m_cdcOnlineEventT0    = std::numeric_limits<float>::quiet_NaN();
  m_cdcOnlineEventT0Err = std::numeric_limits<float>::quiet_NaN();
  m_topEventT0          = std::numeric_limits<float>::quiet_NaN();
  m_topEventT0Err       = std::numeric_limits<float>::quiet_NaN();
  m_topOnlineEventT0    = std::numeric_limits<float>::quiet_NaN();
  m_topOnlineEventT0Err = std::numeric_limits<float>::quiet_NaN();
  m_eclEventT0          = std::numeric_limits<float>::quiet_NaN();
  m_eclEventT0Err       = std::numeric_limits<float>::quiet_NaN();
  m_eclOnlineEventT0    = std::numeric_limits<float>::quiet_NaN();
  m_eclOnlineEventT0Err = std::numeric_limits<float>::quiet_NaN();
  m_eclTCEmax           = std::numeric_limits<int>::quiet_NaN();
  m_eclTCid             = std::numeric_limits<int>::quiet_NaN();

  StoreObjPtr<EventMetaData> evtMetaData;
  m_exp = evtMetaData->getExperiment();
  m_run = evtMetaData->getRun();
  m_event = evtMetaData->getEvent();

  if (m_EventT0.isValid())
    if (m_EventT0->hasEventT0()) {
      m_eventT0 = (float)m_EventT0->getEventT0();
      m_eventT0Err = (float)m_EventT0->getEventT0Uncertainty();

      if (m_EventT0->hasTemporaryEventT0(Const::EDetector::SVD)) {
        auto evtT0List_SVD = m_EventT0->getTemporaryEventT0s(Const::EDetector::SVD) ;
        //    There is only one estimate of SVD EVentT0 for the moment
        m_svdEventT0 = evtT0List_SVD.back().eventT0 ;
        m_svdEventT0Err = evtT0List_SVD.back().eventT0Uncertainty;
      }

      if (m_EventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
        auto evtT0List_CDC = m_EventT0->getTemporaryEventT0s(Const::EDetector::CDC) ;
        //    The most accurate CDC event t0 value is the last one in the list.
        m_cdcEventT0 = evtT0List_CDC.back().eventT0 ;
        m_cdcEventT0Err = evtT0List_CDC.back().eventT0Uncertainty;
      }

      if (m_EventT0->hasTemporaryEventT0(Const::EDetector::TOP)) {
        auto evtT0List_TOP = m_EventT0->getTemporaryEventT0s(Const::EDetector::TOP) ;
        //    There should only be at most one value in the list per event
        m_topEventT0 = evtT0List_TOP.back().eventT0 ;
        m_topEventT0Err = evtT0List_TOP.back().eventT0Uncertainty;
      }

      if (m_EventT0->hasTemporaryEventT0(Const::EDetector::ECL)) {
        auto evtT0List_ECL = m_EventT0->getTemporaryEventT0s(Const::EDetector::ECL) ;
        //    There is only one estimate of ECL EVentT0 for the moment
        m_eclEventT0 = evtT0List_ECL.back().eventT0 ;
        m_eclEventT0Err = evtT0List_ECL.back().eventT0Uncertainty;
      }
    }

  StoreArray<OnlineEventT0> onlineEventT0;
  for (auto& evt : onlineEventT0) {
    if (evt.getOnlineEventT0Detector() == Const::EDetector::SVD) {
      B2DEBUG(40, "OnlineEventT0 given by SVD");
      m_svdOnlineEventT0    = evt.getOnlineEventT0();
      m_svdOnlineEventT0Err = evt.getOnlineEventT0Uncertainty();
    }

    if (evt.getOnlineEventT0Detector() == Const::EDetector::CDC) {
      B2DEBUG(40, "OnlineEventT0 given by CDC");
      m_cdcOnlineEventT0    = evt.getOnlineEventT0();
      m_cdcOnlineEventT0Err = evt.getOnlineEventT0Uncertainty();
    }

    if (evt.getOnlineEventT0Detector() == Const::EDetector::ECL) {
      B2DEBUG(40, "OnlineEventT0 given by ECL");
      m_eclOnlineEventT0    = evt.getOnlineEventT0();
      m_eclOnlineEventT0Err = evt.getOnlineEventT0Uncertainty();
    }

    if (evt.getOnlineEventT0Detector() == Const::EDetector::TOP) {
      B2DEBUG(40, "OnlineEventT0 given by TOP");
      m_topOnlineEventT0    = evt.getOnlineEventT0();
      m_topOnlineEventT0Err = evt.getOnlineEventT0Uncertainty();
    }
  }


  // clear all vectors
  m_trkNumber.clear();
  m_svdisUside.clear();
  m_svdSize.clear();
  m_svdSensor.clear();
  m_svdLadder.clear();
  m_svdLadder.clear();
  m_svdTrkPhi.clear();
  m_svdTrkTheta.clear();
  m_svdTrkpCM.clear();
  m_svdTrkp.clear();
  m_svdTrkpT.clear();
  m_svdTrkz0.clear();
  m_svdTrkd0.clear();
  m_svdTrkPValue.clear();
  m_svdTrkCharge.clear();
  m_svdTrkNDF.clear();
  m_svdTrkCDCHits.clear();
  m_svdTrkSVDHits.clear();
  m_svdTrkPXDHits.clear();
  m_svdTrueTime.clear();
  m_svdClPosErr.clear();
  m_svdClPos.clear();
  m_svdClTimeErr.clear();
  m_svdClTime.clear();
  m_svdClSNR.clear();
  m_svdClCharge.clear();


  //first check SVDEventInfo name
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");
  m_svdTB = eventinfo->getModeByte().getTriggerBin();

  bool isMC = Environment::Instance().isMC();

  int trkNumber = 0;
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  m_nTracks = recoTracks.getEntries();
  for (const auto& trk : recoTracks) {
    if (! trk.wasFitSuccessful()) {
      m_nTracks -= 1;
      continue;
    }


    RelationVector<Track> theTK = DataStore::getRelationsWithObj<Track>(&trk);

    if (theTK.size() == 0) {
      m_nTracks -= 1;
      continue;
    }

    trkNumber ++;

    const vector<SVDCluster* > svdClusters = trk.getSVDHitList();
    B2DEBUG(40, "FITTED TRACK:   NUMBER OF SVD HITS = " << svdClusters.size());

    for (unsigned int i = 0; i < svdClusters.size(); i++) {

      const TrackFitResult*  tfr = theTK[0]->getTrackFitResultWithClosestMass(Const::pion);

      if (tfr) {
        m_svdTrkTheta.push_back(tfr->getMomentum().Theta());
        m_svdTrkPhi.push_back(tfr->getMomentum().Phi());
        m_svdTrkd0.push_back(tfr->getD0());
        m_svdTrkz0.push_back(tfr->getZ0());
        m_svdTrkp.push_back(tfr->getMomentum().R());
        m_svdTrkpT.push_back(tfr->getMomentum().Rho());
        m_svdTrkPValue.push_back(tfr->getPValue());
        m_svdTrkCharge.push_back(tfr->getChargeSign());
        m_svdTrkNDF.push_back(tfr->getNDF());
        ROOT::Math::PxPyPzEVector pStar = tfr->get4Momentum();
        ROOT::Math::BoostZ boost(3. / 11);
        pStar = boost(pStar);
        m_svdTrkpCM.push_back(pStar.P());
      }



      m_svdTrkPXDHits.push_back((trk.getPXDHitList()).size());
      m_svdTrkSVDHits.push_back((trk.getSVDHitList()).size());
      m_svdTrkCDCHits.push_back((trk.getCDCHitList()).size());
      m_trkNumber.push_back(trkNumber);

      const SVDCluster* svd_1 = svdClusters[i];

      //get true hits, used only if isMC
      RelationVector<SVDTrueHit> trueHit_1 = DataStore::getRelationsWithObj<SVDTrueHit>(svd_1);




      const RecoHitInformation* infoSVD_1 = trk.getRecoHitInformation(svd_1);
      if (!infoSVD_1) {
        continue;
      }

      const VxdID svd_id_1 = svd_1->getSensorID();
      m_svdLayer.push_back(svd_id_1.getLayerNumber());
      m_svdLadder.push_back(svd_id_1.getLadderNumber());
      m_svdSensor.push_back(svd_id_1.getSensorNumber());

      m_svdisUside.push_back(svd_1->isUCluster());

      m_svdSize.push_back(svd_1->getSize());

      m_svdClTime.push_back(svd_1->getClsTime());
      m_svdClTimeErr.push_back(svd_1->getClsTimeSigma());
      m_svdClSNR.push_back(svd_1->getSNR());
      m_svdClCharge.push_back(svd_1->getCharge());
      m_svdClPos.push_back(svd_1->getPosition());
      m_svdClPosErr.push_back(svd_1->getPositionSigma());
      if (isMC && trueHit_1.size() > 0) {

        m_svdTrueTime.push_back(trueHit_1[0]->getGlobalTime());

        RelationVector<MCParticle> mcParticle_1 = trueHit_1[0]->getRelationsFrom<MCParticle>();
        if (mcParticle_1.size() > 0) {
          if (mcParticle_1[0]->isPrimaryParticle())
            m_trueEventT0 = mcParticle_1[0]->getProductionTime();
        }
      } else
        m_svdTrueTime.push_back(std::numeric_limits<float>::quiet_NaN());
    }
  }

  StoreArray<TRGECLUnpackerStore> TRGECLData;
  for (const auto& trgHit : TRGECLData) {
    int hitWin   = trgHit.getHitWin();
    B2DEBUG(40, "hitWin = " << hitWin);
    if (hitWin != 3 && hitWin != 4) { continue; }
    if (trgHit.getTCEnergy() > m_eclTCEmax) {
      m_eclTCid      = trgHit.getTCId();
      m_eclTCEmax    = trgHit.getTCEnergy();
    }
  }

  m_t->Fill();

}


void SVDEventT0PerformanceTTreeModule::terminate()
{

  if (m_rootFilePtr != nullptr) {

    TDirectory::TContext context;

    m_rootFilePtr->cd();
    m_t->Write();
    m_rootFilePtr->Close();
  }
}
