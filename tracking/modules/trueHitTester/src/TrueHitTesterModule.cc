/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors:                                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/trueHitTester/TrueHitTesterModule.h>

#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>

#include <framework/datastore/RelationIndex.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <pxd/geometry/SensorInfo.h>

using namespace std;
using namespace Belle2;
//using namespace boost::accumulators;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrueHitTester)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrueHitTesterModule::TrueHitTesterModule() : Module()
{
//Set module properties
  setDescription("tests impact of material effects on tracks with simulated true hits");

//Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("truehittester.root"));
  addParam("only6", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer (will only work with single track events)", false);

}


TrueHitTesterModule::~TrueHitTesterModule()
{

}

void TrueHitTesterModule::initialize()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_nPxdLayers = geo.getLayers(PXD::SensorInfo::PXD).size();
  m_nSvdLayers = geo.getLayers(SVD::SensorInfo::SVD).size();
  m_nLayers = m_nPxdLayers + m_nSvdLayers;

  m_rootFilePtr = new TFile(m_dataOutFileName.c_str(), "RECREATE");
  m_trueHitDataTreePtr = new TTree("m_trueHitDataTreePtr", "aTree");

  registerLayerWiseData("deltaEs", 2);
  registerLayerWiseData("deltaTrackParas", 5);
  registerLayerWiseData("angles", 2);

  m_notPerfectCounter = 0;


}


void TrueHitTesterModule::beginRun()
{

}


void TrueHitTesterModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "********** TrueHitTesterModule  processing event number: " << eventCounter << " ************");
  //simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray("");
  int nMcParticles = aMcParticleArray.getEntries();
  // pointer to the true hits
  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;
  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;
  // class to convert global and local coordinates into each other
  VXD::GeoCache& aGeoCach = VXD::GeoCache::getInstance();

  // if option is set ignore every track that does not have exactly 1 hit in every Si layer
  bool filterEvent = false;
  if (m_filter == true) {
    for (int iPart = 0; iPart not_eq nMcParticles; ++iPart) {
      const MCParticle* const aMcParticlePtr = aMcParticleArray[iPart];
      if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle) == true) {
        vector<int> layerIds;
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
          int layerId = iterPairMcPxd.first->to->getSensorID().getLayerNumber();
          layerIds.push_back(layerId);
          ++iterPairMcPxd.first;
        }
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
          int layerId = iterPairMcSvd.first->to->getSensorID().getLayerNumber();
          layerIds.push_back(layerId);
          ++iterPairMcSvd.first;
        }
        if (layerIds.size() not_eq unsigned(m_nLayers)) {
          filterEvent = true;
          ++m_notPerfectCounter;
          break;
        } else {
          sort(layerIds.begin(), layerIds.end());
          for (int l = 0; l not_eq m_nLayers; ++l) {
            if (l + 1 not_eq layerIds[l]) {
              filterEvent = true;
              ++m_notPerfectCounter;
              break;
            }
          }
        }

      }
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
        vector<VXDTrueHit const*> trueHitPtrs;
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
          trueHitPtrs.push_back(static_cast<VXDTrueHit const*>(iterPairMcPxd.first->to));
          ++iterPairMcPxd.first;
        }
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
          trueHitPtrs.push_back(static_cast<VXDTrueHit const*>(iterPairMcSvd.first->to));
          ++iterPairMcSvd.first;
        }
        int nTrueHits = trueHitPtrs.size();
        B2DEBUG(100, "nTrueHits " << nTrueHits);
        for (int i = 0; i not_eq nTrueHits; ++i) {
          B2DEBUG(100, "i " << i);
          VXDTrueHit const* aVxdTrueHitPtr = trueHitPtrs[i];
          B2DEBUG(100, "aVxdTrueHitPtr " << aVxdTrueHitPtr);
          B2DEBUG(100, "aVxdTrueHitPtr->getU() " << aVxdTrueHitPtr->getU());
          B2DEBUG(100, "aVxdTrueHitPtr->getGlobalTime() " << aVxdTrueHitPtr->getGlobalTime());
          float deltaE = aVxdTrueHitPtr->getEnergyDep();
          int layerId = aVxdTrueHitPtr->getSensorID().getLayerNumber();
          TVector3 pTrueIn = aVxdTrueHitPtr->getEntryMomentum();
          TVector3 pTrueOut = aVxdTrueHitPtr->getExitMomentum();
          TVector3 pTrue = aVxdTrueHitPtr->getMomentum();
          int vecIndex = layerId - 1;

          double energy = sqrt(pTrueOut.Mag() * pTrueOut.Mag() + mass * mass);
          double deltaEFromMom = energyLastHit - energy;
          energyLastHit = energy;
          dataSample.push_back(deltaE);
          dataSample.push_back(deltaEFromMom);
          fillLayerWiseData("deltaEs", vecIndex, dataSample);
          dataSample.clear();


          //TVector3 deltaP = pTrueOut - pTrueIn;
          //pTrueIn.Print();
          //pTrueOut.Print();
          //deltaP.Print();
          const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aVxdTrueHitPtr->getSensorID());
          TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
          TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
          //pTrueInGlobal.Print();
          //pTrueOutGlobal.Print();
          //cout << "q,p " << qTrue << " "; pTrue.Print();
          //cout << "the others"; pTrueIn.Print(); pTrueOut.Print();

          trueState[0][0] = qTrue / pTrue.Mag();
          trueState[1][0] = pTrue[0] / pTrue[2];
          trueState[2][0] = pTrue[1] / pTrue[2];
          trueState[3][0] = aVxdTrueHitPtr->getU();
          trueState[4][0] = aVxdTrueHitPtr->getV();
          trueStateIn[0][0] = qTrue / pTrueIn.Mag();
          trueStateIn[1][0] = pTrueIn[0] / pTrueIn[2];
          trueStateIn[2][0] = pTrueIn[1] / pTrueIn[2];
          trueStateIn[3][0] = aVxdTrueHitPtr->getEntryU();
          trueStateIn[4][0] = aVxdTrueHitPtr->getEntryV();
          trueStateOut[0][0] = qTrue / pTrueOut.Mag();
          trueStateOut[1][0] = pTrueOut[0] / pTrueOut[2];
          trueStateOut[2][0] = pTrueOut[1] / pTrueOut[2];
          trueStateOut[3][0] = aVxdTrueHitPtr->getExitU();
          trueStateOut[4][0] = aVxdTrueHitPtr->getExitV();
          TMatrixT<double> deltaTrueState = trueStateOut - trueStateIn;

          vector<double> deltaTrueStateStdVec(deltaTrueState.GetMatrixArray(), deltaTrueState.GetMatrixArray() + deltaTrueState.GetNrows()); //convert TMatrixD to std::vector

          fillLayerWiseData("deltaTrackParas", vecIndex, deltaTrueStateStdVec);

          dataSample.push_back(pTrueOutGlobal.DeltaPhi(pTrueInGlobal));
          dataSample.push_back(pTrueOutGlobal.Theta() - pTrueInGlobal.Theta());
          fillLayerWiseData("angles", vecIndex, dataSample);
          dataSample.clear();
        }
        m_trueHitDataTreePtr->Fill();
      }

    }
  }
}

void TrueHitTesterModule::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " tracks had not exactly one hit in every layer and were not written to the TTree");
  }
}


void TrueHitTesterModule::terminate()
{
  m_rootFilePtr->cd();
  m_trueHitDataTreePtr->Write();
  m_rootFilePtr->Close();
  // delete all the objects associated with branches
  std::map<std::string, std::vector<vector<float> >* >::iterator iter = m_layerWiseDataForRoot.begin();
  std::map<std::string, std::vector<vector<float> >* >::const_iterator iterMax = m_layerWiseDataForRoot.end();
  while (iter not_eq iterMax) {
    delete(iter->second);
    ++iter;
  }
}



void TrueHitTesterModule::registerLayerWiseData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_layerWiseDataForRoot[nameOfDataSample] = new vector<vector<float> >(m_nLayers, vector<float>(nVarsToTest));
  m_trueHitDataTreePtr->Branch(nameOfDataSample.c_str(), "std::vector<std::vector<float> >", &(m_layerWiseDataForRoot[nameOfDataSample]));
}

void TrueHitTesterModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    (*m_layerWiseDataForRoot[nameOfDataSample])[accuVecIndex][i] = float(newData[i]);
  }
}
