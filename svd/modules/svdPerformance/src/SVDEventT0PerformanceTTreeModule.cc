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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TVector3.h>
#include <TDirectory.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>


using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventT0PerformanceTTree)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventT0PerformanceTTreeModule::SVDEventT0PerformanceTTreeModule() : Module()
{
  //Set module properties
  setDescription("The module is used to create a TTree to study SVD clusters, genfit unbiased residuals and many other properties related to the track they belong to.");
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
  m_eventT0.isOptional();

  TDirectory::TContext context;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //Tree for SVD clusters
  //one fill per event!
  m_t = new TTree("tree", "Tree for SVD clusters related to tracks");
  m_t->Branch("eventT0", &m_svdEventT0, "eventT0/F");
  m_t->Branch("eventT0Err", &m_svdEventT0Err, "eventT0Err/F");
  m_t->Branch("totTracks", &m_nTracks, "totTracks/i");
  m_t->Branch("TB", &m_svdTB, "TB/i");
  m_t->Branch("trkNumb", &m_trkNumber);
  m_t->Branch("trkd0", &m_svdTrkd0);
  m_t->Branch("trkz0", &m_svdTrkz0);
  m_t->Branch("trkp", &m_svdTrkp);
  m_t->Branch("trkpT", &m_svdTrkpT);
  m_t->Branch("trkpCM", &m_svdTrkpCM);
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

  m_svdEventT0 = -99;
  m_svdEventT0Err = -99;

  if (m_eventT0.isValid())
    if (m_eventT0->hasEventT0()) {
      m_svdEventT0 = (float)m_eventT0->getEventT0();
      m_svdEventT0Err = (float)m_eventT0->getEventT0Uncertainty();
    }

  // clear all vectors
  m_trkNumber.clear();
  m_svdisUside.clear();
  m_svdSize.clear();
  m_svdSensor.clear();
  m_svdLadder.clear();
  m_svdLadder.clear();
  m_svdTrkpCM.clear();
  m_svdTrkp.clear();
  m_svdTrkpT.clear();
  m_svdTrkz0.clear();
  m_svdTrkd0.clear();
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
        m_svdTrkd0.push_back(tfr->getD0());
        m_svdTrkz0.push_back(tfr->getZ0());
        m_svdTrkp.push_back(tfr->getMomentum().Mag());
        m_svdTrkpT.push_back(tfr->getMomentum().Perp());
        TLorentzVector pStar = tfr->get4Momentum();
        pStar.Boost(0, 0, 3. / 11);
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
      if (isMC && trueHit_1.size() > 0)
        m_svdTrueTime.push_back(trueHit_1[0]->getGlobalTime());
      else
        m_svdTrueTime.push_back(-99);

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
