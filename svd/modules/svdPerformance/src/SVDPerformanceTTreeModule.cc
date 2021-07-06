/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDPerformanceTTreeModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TVector3.h>
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
REG_MODULE(SVDPerformanceTTree)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDPerformanceTTreeModule::SVDPerformanceTTreeModule() : Module()
{
  //Set module properties
  setDescription("The module is used to create a TTree to study SVD clusters, genfit unbiased residuals and many other properties related to the track they belong to.");
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
  m_t_U->Branch("svdClSNR", &m_svdClSNR, "svdClSNR/F");
  m_t_U->Branch("svdClCharge", &m_svdClCharge, "svdClCharge/F");
  m_t_U->Branch("svdStripCharge", &m_svdStripCharge);
  m_t_U->Branch("svdStrip6Samples", &m_svdStrip6Samples);
  m_t_U->Branch("svdClTime", &m_svdClTime, "svdClTime/F");
  m_t_U->Branch("svdStripTime", &m_svdStripTime);
  m_t_U->Branch("svdStripPosition", &m_svdStripPosition);
  m_t_U->Branch("svdRes", &m_svdRes, "svdRes/F");
  m_t_U->Branch("svdPitch", &m_svdPitch, "svdPitch/F");
  m_t_U->Branch("svdWidth", &m_svdWidth, "svdWidth/F");
  m_t_U->Branch("svdLength", &m_svdLength, "svdLength/F");
  m_t_U->Branch("svdClIntStrPos", &m_svdClIntStrPos, "svdClIntStrPos/F");
  m_t_U->Branch("svdClPos", &m_svdClPos, "svdClPos/F");
  m_t_U->Branch("svdClPosErr", &m_svdClPosErr, "svdClPosErr/F");
  m_t_U->Branch("svdTruePos", &m_svdTruePos, "svdTruePos/F");
  m_t_U->Branch("svdClPhi", &m_svdClPhi, "svdClPhi/F");
  m_t_U->Branch("svdClZ", &m_svdClZ, "svdClZ/F");
  m_t_U->Branch("svdTrkd0", &m_svdTrkd0, "svdTrkd0/F");
  m_t_U->Branch("svdTrkz0", &m_svdTrkz0, "svdTrkz0/F");
  m_t_U->Branch("svdTrkpT", &m_svdTrkpT, "svdTrkpT/F");
  m_t_U->Branch("svdTrkpCM", &m_svdTrkpCM, "svdTrkpCM/F");
  m_t_U->Branch("svdTrkTraversedLength", &m_svdTrkTraversedLength, "svdTrkTraversedLength/F");
  m_t_U->Branch("svdTrkPXDHits", &m_svdTrkPXDHits, "svdTrkPXDHits/i");
  m_t_U->Branch("svdTrkSVDHits", &m_svdTrkSVDHits, "svdTrkSVDHits/i");
  m_t_U->Branch("svdTrkCDCHits", &m_svdTrkCDCHits, "svdTrkCDCHits/i");
  m_t_U->Branch("svdTrkPos", &m_svdTrkPos, "svdTrkPos/F");
  m_t_U->Branch("svdTrkPosOS", &m_svdTrkPosOS, "svdTrkPosOS/F");
  m_t_U->Branch("svdTrkPosErr", &m_svdTrkPosErr, "svdTrkPosErr/F");
  m_t_U->Branch("svdTrkPosErrOS", &m_svdTrkPosErrOS, "svdTrkPosErrOS/F");
  m_t_U->Branch("svdTrkQoP", &m_svdTrkQoP, "svdTrkQoP/F");
  m_t_U->Branch("svdTrkPrime", &m_svdTrkPrime, "svdTrkPrime/F");
  m_t_U->Branch("svdTrkPrimeOS", &m_svdTrkPrimeOS, "svdTrkPrimeOS/F");
  m_t_U->Branch("svdTrkPosUnbiased", &m_svdTrkPosUnbiased, "svdTrkPosUnbiased/F");
  m_t_U->Branch("svdTrkPosErrUnbiased", &m_svdTrkPosErrUnbiased, "svdTrkPosErrUnbiased/F");
  m_t_U->Branch("svdTrkQoPUnbiased", &m_svdTrkQoPUnbiased, "svdTrkQoPUnbiased/F");
  m_t_U->Branch("svdTrkPrimeUnbiased", &m_svdTrkPrimeUnbiased, "svdTrkPrimeUnbiased/F");
  m_t_U->Branch("svdLayer", &m_svdLayer, "svdLayer/i");
  m_t_U->Branch("svdLadder", &m_svdLadder, "svdLadder/i");
  m_t_U->Branch("svdSensor", &m_svdSensor, "svdSensor/i");
  m_t_U->Branch("svdSize", &m_svdSize, "svdSize/i");
  m_t_U->Branch("svdTB", &m_svdTB, "svdTB/i");
  //Tree for SVD v overlapping clusters
  m_t_V = new TTree("t_V", "Tree for M_SVD v-clusters");
  m_t_V->Branch("svdClSNR", &m_svdClSNR, "svdClSNR/F");
  m_t_V->Branch("svdClCharge", &m_svdClCharge, "svdClCharge/F");
  m_t_V->Branch("svdStripCharge", &m_svdStripCharge);
  m_t_V->Branch("svdStrip6Samples", &m_svdStrip6Samples);
  m_t_V->Branch("svdClTime", &m_svdClTime, "svdClTime/F");
  m_t_V->Branch("svdStripTime", &m_svdStripTime);
  m_t_V->Branch("svdStripPosition", &m_svdStripPosition);
  m_t_V->Branch("svdRes", &m_svdRes, "svdRes/F");
  m_t_V->Branch("svdPitch", &m_svdPitch, "svdPitch/F");
  m_t_V->Branch("svdWidth", &m_svdWidth, "svdWidth/F");
  m_t_V->Branch("svdLength", &m_svdLength, "svdLength/F");
  m_t_V->Branch("svdClIntStrPos", &m_svdClIntStrPos, "svdClIntStrPos/F");
  m_t_V->Branch("svdClPos", &m_svdClPos, "svdClPos/F");
  m_t_V->Branch("svdClPosErr", &m_svdClPosErr, "svdClPosErr/F");
  m_t_V->Branch("svdTruePos", &m_svdTruePos, "svdTruePos/F");
  m_t_V->Branch("svdClPhi", &m_svdClPhi, "svdClPhi/F");
  m_t_V->Branch("svdClZ", &m_svdClZ, "svdClZ/F");
  m_t_V->Branch("svdTrkd0", &m_svdTrkd0, "svdTrkd0/F");
  m_t_V->Branch("svdTrkz0", &m_svdTrkz0, "svdTrkz0/F");
  m_t_V->Branch("svdTrkpT", &m_svdTrkpT, "svdTrkpT/F");
  m_t_V->Branch("svdTrkpCM", &m_svdTrkpCM, "svdTrkpCM/F");
  m_t_V->Branch("svdTrkTraversedLength", &m_svdTrkTraversedLength, "svdTrkTraversedLength/F");
  m_t_V->Branch("svdTrkPXDHits", &m_svdTrkPXDHits, "svdTrkPXDHits/i");
  m_t_V->Branch("svdTrkSVDHits", &m_svdTrkSVDHits, "svdTrkSVDHits/i");
  m_t_V->Branch("svdTrkCDCHits", &m_svdTrkCDCHits, "svdTrkCDCHits/i");
  m_t_V->Branch("svdTrkPos", &m_svdTrkPos, "svdTrkPos/F");
  m_t_V->Branch("svdTrkPosOS", &m_svdTrkPosOS, "svdTrkPosOS/F");
  m_t_V->Branch("svdTrkPosErr", &m_svdTrkPosErr, "svdTrkPosErr/F");
  m_t_V->Branch("svdTrkPosErrOS", &m_svdTrkPosErrOS, "svdTrkPosErrOS/F");
  m_t_V->Branch("svdTrkQoP", &m_svdTrkQoP, "svdTrkQoP/F");
  m_t_V->Branch("svdTrkPrime", &m_svdTrkPrime, "svdTrkPrime/F");
  m_t_V->Branch("svdTrkPrimeOS", &m_svdTrkPrimeOS, "svdTrkPrimeOS/F");
  m_t_V->Branch("svdTrkPosUnbiased", &m_svdTrkPosUnbiased, "svdTrkPosUnbiased/F");
  m_t_V->Branch("svdTrkPosErrUnbiased", &m_svdTrkPosErrUnbiased, "svdTrkPosErrUnbiased/F");
  m_t_V->Branch("svdTrkQoPUnbiased", &m_svdTrkQoPUnbiased, "svdTrkQoPUnbiased/F");
  m_t_V->Branch("svdTrkPrimeUnbiased", &m_svdTrkPrimeUnbiased, "svdTrkPrimeUnbiased/F");
  m_t_V->Branch("svdLayer", &m_svdLayer, "svdLayer/i");
  m_t_V->Branch("svdLadder", &m_svdLadder, "svdLadder/i");
  m_t_V->Branch("svdSensor", &m_svdSensor, "svdSensor/i");
  m_t_V->Branch("svdSize", &m_svdSize, "svdSize/i");
  m_t_V->Branch("svdTB", &m_svdTB, "svdTB/i");

}

void SVDPerformanceTTreeModule::event()
{

  //first check SVDEventInfo name
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");
  m_svdTB = eventinfo->getModeByte().getTriggerBin();

  bool isMC = Environment::Instance().isMC();

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  for (const auto& trk : recoTracks) {
    if (! trk.wasFitSuccessful()) {
      continue;
    }


    RelationVector<Track> theTK = DataStore::getRelationsWithObj<Track>(&trk);

    if (theTK.size() == 0) {
      continue;
    }


    const TrackFitResult*  tfr = theTK[0]->getTrackFitResultWithClosestMass(Const::pion);

    if (tfr) {
      m_svdTrkd0 = tfr->getD0();
      m_svdTrkz0 = tfr->getZ0();
      m_svdTrkpT = tfr->getMomentum().Perp();
      TLorentzVector pStar = tfr->get4Momentum();
      pStar.Boost(0, 0, 3. / 11);
      m_svdTrkpCM = pStar.P();
    }


    const vector<SVDCluster* > svdClusters = trk.getSVDHitList();
    B2DEBUG(40, "FITTED TRACK:   NUMBER OF SVD HITS = " << svdClusters.size());

    m_svdTrkPXDHits = (trk.getPXDHitList()).size();
    m_svdTrkSVDHits = (trk.getSVDHitList()).size();
    m_svdTrkCDCHits = (trk.getCDCHitList()).size();

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

      try {
        const TVectorD resUnBias_1 =  fittedResult_1->getResidual(0, false).getState();
        genfit::MeasuredStateOnPlane state_unbiased = fittedResult_1->getFittedState(false);
        const TVectorD& svd_predIntersect_unbiased = state_unbiased.getState();
        const TMatrixDSym& covMatrix_unbiased = state_unbiased.getCov();
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
          m_svdClPosErr = svd_1->getPositionSigma();
          if (isMC && trueHit_1.size() > 0)
            m_svdTruePos = trueHit_1[0]->getU();
          else
            m_svdTruePos = -99;
          m_svdClPhi = svdPhi_1;
          m_svdClZ = svdZ_1;
          m_svdTrkPos = svd_predIntersect_1[3];
          m_svdTrkPosOS = svd_predIntersect_1[4];
          m_svdTrkPosErr = sqrt(covMatrix_1[3][3]);
          m_svdTrkPosErrOS = sqrt(covMatrix_1[4][4]);
          m_svdTrkQoP = svd_predIntersect_1[0];
          m_svdTrkPrime = svd_predIntersect_1[1];
          m_svdTrkPrimeOS = svd_predIntersect_1[2];
          m_svdTrkTraversedLength = svdSensor_1.getThickness() * sqrt(1 + m_svdTrkPrimeOS * m_svdTrkPrimeOS + m_svdTrkPrime * m_svdTrkPrime);
          m_svdTrkPosUnbiased = svd_predIntersect_unbiased[3];
          m_svdClPos = m_svdRes / 1e4 + m_svdTrkPosUnbiased;
          m_svdTrkPosErrUnbiased = sqrt(covMatrix_unbiased[3][3]);
          m_svdTrkQoPUnbiased = svd_predIntersect_unbiased[0];
          m_svdTrkPrimeUnbiased = svd_predIntersect_unbiased[1];
          m_svdLayer = svd_Layer_1;
          m_svdLadder = svd_Ladder_1;
          m_svdSensor = svd_Sensor_1;
          m_svdSize = strips_1;

          m_svdPitch = svdSensor_1.getUPitch(m_svdTrkPosOS);
          m_svdWidth = svdSensor_1.getUSize(m_svdTrkPosOS);
          m_svdLength = svdSensor_1.getVSize();

          m_svdClIntStrPos = fmod(m_svdClPos + 0.5 * m_svdWidth, m_svdPitch) / m_svdPitch;

          m_svdStripCharge.clear();
          m_svdStripTime.clear();
          m_svdStripPosition.clear();
          m_svdStrip6Samples.clear();
          //retrieve relations and set strip charges and times
          RelationVector<SVDRecoDigit> theRecoDigits = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_1);
          if ((theRecoDigits.size() != m_svdSize) && (m_svdSize != 128)) //virtual cluster
            B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits.size() << " != " << m_svdSize << " cluster size");

          //skip clusters created beacuse of missing APV
          if (m_svdSize < 128)
            for (unsigned int d = 0; d < m_svdSize; d++) {

              SVDShaperDigit* ShaperDigit = theRecoDigits[d]->getRelated<SVDShaperDigit>();
              array<float, 6> Samples = ShaperDigit->getSamples();

              m_svdStripCharge.push_back(theRecoDigits[d]->getCharge());
              std::copy(std::begin(Samples), std::end(Samples), std::back_inserter(m_svdStrip6Samples));
              m_svdStripTime.push_back(theRecoDigits[d]->getTime());
              double misalignedStripPos = svdSensor_1.getUCellPosition(theRecoDigits[d]->getCellID());
              //aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
              m_svdStripPosition.push_back(misalignedStripPos - svd_1->getPosition() + m_svdClPos);
            }



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
          m_svdClPosErr = svd_1->getPositionSigma();
          if (isMC && trueHit_1.size() > 0)
            m_svdTruePos = trueHit_1[0]->getV();
          else
            m_svdTruePos = -99;
          m_svdClPhi = svdPhi_1;
          m_svdClZ = svdZ_1;
          m_svdTrkPos = svd_predIntersect_1[4];
          m_svdTrkPosOS = svd_predIntersect_1[3];
          m_svdTrkPosErr = sqrt(covMatrix_1[4][4]);
          m_svdTrkPosErrOS = sqrt(covMatrix_1[3][3]);
          m_svdTrkQoP = svd_predIntersect_1[0];
          m_svdTrkPrime = svd_predIntersect_1[2];
          m_svdTrkPrimeOS = svd_predIntersect_1[1];
          m_svdTrkTraversedLength = svdSensor_1.getThickness() * sqrt(1 + m_svdTrkPrimeOS * m_svdTrkPrimeOS + m_svdTrkPrime * m_svdTrkPrime);
          m_svdTrkPosUnbiased = svd_predIntersect_unbiased[4];
          m_svdClPos = m_svdRes / 1e4 + m_svdTrkPosUnbiased;
          m_svdTrkPosErrUnbiased = sqrt(covMatrix_unbiased[4][4]);
          m_svdTrkQoPUnbiased = svd_predIntersect_unbiased[0];
          m_svdTrkPrimeUnbiased = svd_predIntersect_unbiased[2];
          m_svdLayer = svd_Layer_1;
          m_svdLadder = svd_Ladder_1;
          m_svdSensor = svd_Sensor_1;
          m_svdSize = strips_1;

          m_svdPitch = svdSensor_1.getVPitch();
          m_svdWidth = svdSensor_1.getUSize(m_svdTrkPos);
          m_svdLength = svdSensor_1.getVSize();

          m_svdClIntStrPos = fmod(m_svdClPos + 0.5 * m_svdLength, m_svdPitch) / m_svdPitch;

          m_svdStripCharge.clear();
          m_svdStripTime.clear();
          m_svdStripPosition.clear();
          m_svdStrip6Samples.clear();
          //retrieve relations and set strip charges and times
          RelationVector<SVDRecoDigit> theRecoDigits = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_1);
          if ((theRecoDigits.size() != m_svdSize) && (m_svdSize != 128)) //virtual cluster
            B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits.size() << " != " << m_svdSize << " cluster size");

          //skip clusters created beacuse of missing APV
          if (m_svdSize < 128)
            for (unsigned int d = 0; d < m_svdSize; d++) {
              SVDShaperDigit* ShaperDigit = theRecoDigits[d]->getRelated<SVDShaperDigit>();
              array<float, 6> Samples = ShaperDigit->getSamples();
              m_svdStripCharge.push_back(theRecoDigits[d]->getCharge());
              std::copy(std::begin(Samples), std::end(Samples), std::back_inserter(m_svdStrip6Samples));
              m_svdStripTime.push_back(theRecoDigits[d]->getTime());
              double misalignedStripPos = svdSensor_1.getVCellPosition(theRecoDigits[d]->getCellID());
              //Aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
              m_svdStripPosition.push_back(misalignedStripPos - svd_1->getPosition() + m_svdClPos);
            }

          m_t_V->Fill();
        }
      } catch (...) {
        B2INFO("oops...something went wrong in getting the unbiased state, skipping this cluster.");
        continue;
      }

    }
  }
}


void SVDPerformanceTTreeModule::terminate()
{

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();
    m_t_U->Write();
    m_t_V->Write();
    m_rootFilePtr->Close();
  }
}
