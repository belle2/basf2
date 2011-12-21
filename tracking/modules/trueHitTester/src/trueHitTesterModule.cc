/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors:                                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/trueHitTester/trueHitTesterModule.h>

#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>

#include <framework/datastore/RelationIndex.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/VxdID.h>

using namespace std;
using namespace Belle2;
//using namespace boost::accumulators;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trueHitTester)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trueHitTesterModule::trueHitTesterModule() : Module()
{
//Set module properties
  setDescription("tests impact of material effects on tracks with simulated true hits");

//Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("truehittester.root"));
  addParam("filter", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer (will only work with single track events)", false);

}


trueHitTesterModule::~trueHitTesterModule()
{

}

void trueHitTesterModule::initialize()
{
  m_nPxdLayers = 2;
  m_nSvdLayers = 4;
  m_nLayers = m_nPxdLayers + m_nSvdLayers;

  m_rootFilePtr = new TFile(m_dataOutFileName.c_str(), "RECREATE");
  m_trueHitDataTreePtr = new TTree("m_trueHitDataTreePtr", "aTree");

  registerLayerWiseData("deltaEs", 2);
  registerLayerWiseData("deltaTrackParas", 5);

  m_notPerfectCounter = 0;


}


void trueHitTesterModule::beginRun()
{

}


void trueHitTesterModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();

//simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray("");
  int nMcParticles = aMcParticleArray.getEntries();

  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  int nPxdTrueHits = aPxdTrueHitArray.getEntries();
//cout << "nPxdTrueHits " <<  nPxdTrueHits << "\n";
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  int nSvdTrueHits = aSvdTrueHitArray.getEntries();
//cout << "nSvdTrueHits " <<  nSvdTrueHits << "\n";
  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;
  int sizeRelMcPxdTrueHit = relMcPxdTrueHit.size();
  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;
  int sizeRelMcSvdTrueHit = relMcSvdTrueHit.size();

  //Filter
  bool filterEvent = false;
  if (m_filter == true) {
    // for the filter function to get only tracks that hits specific layers
    int nHitsL1 = 0; int nHitsL2 = 0; int nHitsL3 = 0; int nHitsL4 = 0; int nHitsL5 = 0; int nHitsL6 = 0;
    for (int i = 0; i not_eq nPxdTrueHits; ++i) {
      int layerId = aPxdTrueHitArray[i]->getSensorID().getLayer();
      if (layerId == 1) {
        ++nHitsL1;
      } else {
        ++nHitsL2;
      }
    }
    for (int i = 0; i not_eq nSvdTrueHits; ++i) {
      int layerId = aSvdTrueHitArray[i]->getSensorID().getLayer();
      if (layerId == 3) {
        ++nHitsL3;
      } else if (layerId == 4) {
        ++nHitsL4;
      } else if (layerId == 5) {
        ++nHitsL5;
      } else {
        ++nHitsL6;
      }
    }
    if (nHitsL1 not_eq 1 or nHitsL2 not_eq 1 or nHitsL3 not_eq 1 or nHitsL4 not_eq 1 or nHitsL5 not_eq 1 or nHitsL6 not_eq 1) {
      filterEvent = true;
      ++m_notPerfectCounter;
    }
  }
  if (filterEvent == false) {
    TMatrixT<double> trueStateIn(5, 1);
    TMatrixT<double> trueState(5, 1);
    TMatrixT<double> trueStateOut(5, 1);

    TMatrixT<double> trueStateInGlobal(5, 1);
    TMatrixT<double> trueStateGlobal(5, 1);
    TMatrixT<double> trueStateOutGlobal(5, 1);

    vector<double> dataSample;
    for (int iPart = 0; iPart not_eq nMcParticles; ++iPart) {
      const MCParticle* const aMcParticlePtr = aMcParticleArray[iPart];
      if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle) == true) {
        double qTrue = aMcParticlePtr->getCharge();
        double mass = aMcParticlePtr->getMass();
        //double pVertex = aMcParticlePtr->getMomentum();
        double energyLastHit = aMcParticlePtr->getEnergy() + mass;
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getFrom(aMcParticlePtr);
        while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
          const PXDTrueHit* const aSiTrueHitPtr = iterPairMcPxd.first->to;
          float deltaE = aSiTrueHitPtr->getEnergyDep();
          int layerId = aSiTrueHitPtr->getSensorID().getLayer();
          TVector3 pTrueIn = aSiTrueHitPtr->getEntryMomentum();
          TVector3 pTrueOut = aSiTrueHitPtr->getExitMomentum();
          TVector3 pTrue = aSiTrueHitPtr->getMomentum();
          int vecIndex = layerId - 1;
          double energy = sqrt(pTrueOut.Mag() * pTrueOut.Mag() + mass * mass);
          double deltaEFromMom = energyLastHit - energy;
          energyLastHit = energy;
          dataSample.push_back(deltaE);
          dataSample.push_back(deltaEFromMom);
          fillLayerWiseData("deltaEs", vecIndex, dataSample);
          dataSample.clear();
          //VxdID aSensorId = aSiTrueHitPtr->getSensorID();
          //TVector3 deltaP = pTrueOut - pTrueIn;
          //pTrueIn.Print();
          //pTrueOut.Print();
          //deltaP.Print();
          //const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aSensorId);
          //TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
          //TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
          //pTrueInGlobal.Print();
          //pTrueOutGlobal.Print();
          //cout << "q,p " << qTrue << " "; pTrue.Print();
          //cout << "the others"; pTrueIn.Print(); pTrueOut.Print();
          trueState[0][0] = qTrue / pTrue.Mag();
          trueState[1][0] = pTrue[0] / pTrue[2];
          trueState[2][0] = pTrue[1] / pTrue[2];
          trueState[3][0] = aSiTrueHitPtr->getU();
          trueState[4][0] = aSiTrueHitPtr->getV();
          trueStateIn[0][0] = qTrue / pTrueIn.Mag();
          trueStateIn[1][0] = pTrueIn[0] / pTrueIn[2];
          trueStateIn[2][0] = pTrueIn[1] / pTrueIn[2];
          trueStateOut[0][0] = qTrue / pTrueOut.Mag();
          trueStateOut[1][0] = pTrueOut[0] / pTrueOut[2];
          trueStateOut[2][0] = pTrueOut[1] / pTrueOut[2];
          TMatrixT<double> deltaTrueState = trueStateOut - trueStateIn;
          fillLayerWiseData("deltaTrackParas", vecIndex, rootVecToStdVec(deltaTrueState));
          ++iterPairMcPxd.first;
        }
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getFrom(aMcParticlePtr);
        while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
          const SVDTrueHit* const aSiTrueHitPtr = iterPairMcSvd.first->to;
          float deltaE = aSiTrueHitPtr->getEnergyDep();
          int layerId = aSiTrueHitPtr->getSensorID().getLayer();
          TVector3 pTrueIn = aSiTrueHitPtr->getEntryMomentum();
          TVector3 pTrueOut = aSiTrueHitPtr->getExitMomentum();
          TVector3 pTrue = aSiTrueHitPtr->getMomentum();
          int vecIndex = layerId - 1;
          double energy = sqrt(pTrueOut.Mag() * pTrueOut.Mag() + mass * mass);
          double deltaEFromMom = energyLastHit - energy;
          energyLastHit = energy;
          dataSample.push_back(deltaE);
          dataSample.push_back(deltaEFromMom);
          fillLayerWiseData("deltaEs", vecIndex, dataSample);
          dataSample.clear();
          trueState[0][0] = qTrue / pTrue.Mag();
          trueState[1][0] = pTrue[0] / pTrue[2];
          trueState[2][0] = pTrue[1] / pTrue[2];
          trueState[3][0] = aSiTrueHitPtr->getU();
          trueState[4][0] = aSiTrueHitPtr->getV();
          trueStateIn[0][0] = qTrue / pTrueIn.Mag();
          trueStateIn[1][0] = pTrueIn[0] / pTrueIn[2];
          trueStateIn[2][0] = pTrueIn[1] / pTrueIn[2];
          trueStateOut[0][0] = qTrue / pTrueOut.Mag();
          trueStateOut[1][0] = pTrueOut[0] / pTrueOut[2];
          trueStateOut[2][0] = pTrueOut[1] / pTrueOut[2];
          TMatrixT<double> deltaTrueState = trueStateOut - trueStateIn;
          fillLayerWiseData("deltaTrackParas", vecIndex, rootVecToStdVec(deltaTrueState));
          ++iterPairMcSvd.first;
        }
        m_trueHitDataTreePtr->Fill();
      }

    }
  }
}

void trueHitTesterModule::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " tracks had not exactly on hit in every layer and were not written to the TTree");
  }
}


void trueHitTesterModule::terminate()
{
  m_trueHitDataTreePtr->Write();

  m_rootFilePtr->Close();
  // make a delete for every new in this module
  delete m_trueHitDataTreePtr;
  delete m_rootFilePtr;
  std::map<std::string, LayerWiseData* >::iterator iter = m_layerWiseDataForRoot.begin();
  std::map<std::string, LayerWiseData* >::const_iterator iterMax = m_layerWiseDataForRoot.end();
  while (iter not_eq iterMax) {
    delete(iter->second);
    ++iter;
  }
}



void trueHitTesterModule::registerLayerWiseData(const string& nameOfDataSample, const int nVarsToTest)
{

  m_layerWiseDataForRoot[nameOfDataSample] = new Belle2::LayerWiseData(m_nLayers, nVarsToTest);
  m_trueHitDataTreePtr->Bronch(nameOfDataSample.c_str(), "Belle2::LayerWiseData", &(m_layerWiseDataForRoot[nameOfDataSample]));

}

void trueHitTesterModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_layerWiseDataForRoot[nameOfDataSample]->layerVecData[accuVecIndex][i] = float(newData[i]);
  }
}

void trueHitTesterModule::fillTrackWiseVecData(const string& nameOfDataSample, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_trackWiseVecDataForRoot[nameOfDataSample]->at(i) = float(newData[i]);
  }

}

void trueHitTesterModule::fillTrackWiseData(const string& nameOfDataSample, const double newData)
{

  *(m_trackWiseDataForRoot[nameOfDataSample]) = float(newData);

}

vector<double> trueHitTesterModule::rootVecToStdVec(TMatrixT<double>&  rootVector)
{
  int n = rootVector.GetNrows();
  vector<double> stdVec(n);
  for (int i = 0; i not_eq n; ++i) {
    stdVec[i] = rootVector[i][0];
  }
  return stdVec;
}

