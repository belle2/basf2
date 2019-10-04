/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *
 * Module to study VXD hits from overlapping sensors of a same VXD layer. *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/SVDPerformanceTTreeModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TVector3.h>
#include <TDirectory.h>
#include <math.h>
#include <iostream>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDPerformanceTTree)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDPerformanceTTreeModule::SVDPerformanceTTreeModule() : Module()
{
  //Set module properties
  setDescription("The module studies SVD clusters, genfit unbiased residuals and many other properties related to the track they belong to.");
  //Parameter to take only specific RecoTracks as input
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDPerformanceTTree.root"));
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "StoreArray name of the input and output RecoTracks.",
           m_recoTracksStoreArrayName);
}

void SVDPerformanceTTreeModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  recoTracks.isOptional();

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //Tree for SVD u overlapping clusters
  m_t_U = new TTree("t_U", "Tree for SVD u-clusters");
  m_t_U->Branch("svdClCharge", &m_svdClCharge, "svdClCharge/F");
  m_t_U->Branch("svdClSNR", &m_svdClSNR, "svdClSNR/F");
  m_t_U->Branch("svdClTime", &m_svdClTime, "svdClTime/F");
  m_t_U->Branch("svdRes", &m_svdRes, "svdRes/F");
  m_t_U->Branch("svdClPos", &m_svdClPos, "svdClPos/F");
  m_t_U->Branch("svdClPosErr", &m_svdClPosErr, "svdClPosErr/F");
  m_t_U->Branch("svdTruePos", &m_svdTruePos, "svdTruePos/F");
  m_t_U->Branch("svdClPhi", &m_svdClPhi, "svdClPhi/F");
  m_t_U->Branch("svdClZ", &m_svdClZ, "svdClZ/F");
  m_t_U->Branch("svdTrkPos", &m_svdTrkPos, "svdTrkPos/F");
  m_t_U->Branch("svdTrkPosErr", &m_svdTrkPosErr, "svdTrkPosErr/F");
  m_t_U->Branch("svdTrkQoP", &m_svdTrkQoP, "svdTrkQoP/F");
  m_t_U->Branch("svdTrkPrime", &m_svdTrkPrime, "svdTrkPrime/F");
  m_t_U->Branch("svdLayer", &m_svdLayer, "svdLayer/i");
  m_t_U->Branch("svdLadder", &m_svdLadder, "svdLadder/i");
  m_t_U->Branch("svdSensor", &m_svdSensor, "svdSensor/i");
  m_t_U->Branch("svdSize", &m_svdSize, "svdSize/i");
  //Tree for SVD v overlapping clusters
  m_t_V = new TTree("t_V", "Tree for M_SVD v-clusters");
  m_t_V->Branch("svdClCharge", &m_svdClCharge, "svdClCharge/F");
  m_t_V->Branch("svdClSNR", &m_svdClSNR, "svdClSNR/F");
  m_t_V->Branch("svdClTime", &m_svdClTime, "svdClTime/F");
  m_t_V->Branch("svdRes", &m_svdRes, "svdRes/F");
  m_t_V->Branch("svdClPos", &m_svdClPos, "svdClPos/F");
  m_t_V->Branch("svdClPosErr", &m_svdClPosErr, "svdClPosErr/F");
  m_t_V->Branch("svdTruePos", &m_svdTruePos, "svdTruePos/F");
  m_t_V->Branch("svdClPhi", &m_svdClPhi, "svdClPhi/F");
  m_t_V->Branch("svdClZ", &m_svdClZ, "svdClZ/F");
  m_t_V->Branch("svdTrkPos", &m_svdTrkPos, "svdTrkPos/F");
  m_t_V->Branch("svdTrkPosErr", &m_svdTrkPosErr, "svdTrkPosErr/F");
  m_t_V->Branch("svdTrkQoP", &m_svdTrkQoP, "svdTrkQoP/F");
  m_t_V->Branch("svdTrkPrime", &m_svdTrkPrime, "svdTrkPrime/F");
  m_t_V->Branch("svdLayer", &m_svdLayer, "svdLayer/i");
  m_t_V->Branch("svdLadder", &m_svdLadder, "svdLadder/i");
  m_t_V->Branch("svdSensor", &m_svdSensor, "svdSensor/i");
  m_t_V->Branch("svdSize", &m_svdSize, "svdSize/i");

}

void SVDPerformanceTTreeModule::event()
{
  bool isMC = Environment::Instance().isMC();

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  for (const auto& trk : recoTracks) {
    if (! trk.wasFitSuccessful()) {
      continue;
    }
    const vector<SVDCluster* > svdClusters = trk.getSVDHitList();
    B2DEBUG(40, "FITTED TRACK:   NUMBER OF SVD HITS = " << svdClusters.size());

    for (unsigned int i = 0; i < svdClusters.size(); i++) {

      const SVDCluster* svd_1 = svdClusters[i];

      //get true hits, used only if isMC
      RelationVector<SVDTrueHit> trueHit_1 = DataStore::getRelationsWithObj<SVDTrueHit>(svd_1);

      const RecoHitInformation* infoSVD_1 = trk.getRecoHitInformation(svd_1);
      if (!infoSVD_1) {
        continue;
      }
      const auto* hitTrackPoint_1 = trk.getCreatedTrackPoint(infoSVD_1);
      const auto* fittedResult_1 = hitTrackPoint_1->getFitterInfo();
      if (!fittedResult_1) {
        continue;
      }
      const VxdID svd_id_1 = svd_1->getSensorID();
      const unsigned short svd_Layer_1 = svd_id_1.getLayerNumber();
      const unsigned short svd_Ladder_1 = svd_id_1.getLadderNumber();
      const unsigned short svd_Sensor_1 = svd_id_1.getSensorNumber();

      const TVectorD resUnBias_1 =  fittedResult_1->getResidual(0, false).getState();
      genfit::MeasuredStateOnPlane state_1 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_1);
      const TVectorD& svd_predIntersect_1 = state_1.getState();
      const TMatrixDSym& covMatrix_1 = state_1.getCov();

      if (svd_1->isUCluster()) {

        const int strips_1 = svd_1->getSize();

        const double res_U_1 = resUnBias_1.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
        const TVector3 svdLocal_1(svd_1->getPosition(), svd_predIntersect_1[4], 0.);
        const VXD::SensorInfoBase& svdSensor_1 = geo.get(svd_id_1);
        const TVector3& svdGlobal_1 = svdSensor_1.pointToGlobal(svdLocal_1);
        double svdPhi_1 = atan2(svdGlobal_1(1), svdGlobal_1(0));
        double svdZ_1 = svdGlobal_1(2);

        //Fill SVD tree for u-overlaps if required by the user
        m_svdRes = res_U_1;
        m_svdClTime = svd_1->getClsTime();
        m_svdClSNR = svd_1->getSNR();
        m_svdClCharge = svd_1->getCharge();
        m_svdClPos = svd_1->getPosition();
        m_svdClPosErr = svd_1->getPositionSigma();
        if (isMC && trueHit_1.size() > 0)
          m_svdTruePos = trueHit_1[0]->getU();
        else
          m_svdTruePos = -99;
        m_svdClPhi = svdPhi_1;
        m_svdClZ = svdZ_1;
        m_svdTrkPos = svd_predIntersect_1[3];
        m_svdTrkPosErr = sqrt(covMatrix_1[3][3]);
        m_svdTrkQoP = svd_predIntersect_1[0];
        m_svdTrkPrime = svd_predIntersect_1[1];
        m_svdLayer = svd_Layer_1;
        m_svdLadder = svd_Ladder_1;
        m_svdSensor = svd_Sensor_1;
        m_svdSize = strips_1;
        m_t_U->Fill();
      } else {
        const int strips_1 = svd_1->getSize();
        const double res_V_1 = resUnBias_1.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
        const TVector3 svdLocal_1(svd_predIntersect_1[3], svd_1->getPosition(), 0.);
        const VXD::SensorInfoBase& svdSensor_1 = geo.get(svd_id_1);
        const TVector3& svdGlobal_1 = svdSensor_1.pointToGlobal(svdLocal_1);
        double svdPhi_1 = atan2(svdGlobal_1(1), svdGlobal_1(0));
        double svdZ_1 = svdGlobal_1(2);

        m_svdRes = res_V_1;
        m_svdClTime = svd_1->getClsTime();
        m_svdClSNR = svd_1->getSNR();
        m_svdClCharge = svd_1->getCharge();
        m_svdClPos = svd_1->getPosition();
        m_svdClPosErr = svd_1->getPositionSigma();
        if (isMC && trueHit_1.size() > 0)
          m_svdTruePos = trueHit_1[0]->getV();
        else
          m_svdTruePos = -99;
        m_svdClPhi = svdPhi_1;
        m_svdClZ = svdZ_1;
        m_svdTrkPos = svd_predIntersect_1[4];
        m_svdTrkPosErr = sqrt(covMatrix_1[4][4]);
        m_svdTrkQoP = svd_predIntersect_1[0];
        m_svdTrkPrime = svd_predIntersect_1[2];
        m_svdLayer = svd_Layer_1;
        m_svdLadder = svd_Ladder_1;
        m_svdSensor = svd_Sensor_1;
        m_svdSize = strips_1;
        m_t_V->Fill();
      }
    }
  }
}


void SVDPerformanceTTreeModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    m_t_U->Write();
    m_t_V->Write();
    m_rootFilePtr->Close();
  }
}
