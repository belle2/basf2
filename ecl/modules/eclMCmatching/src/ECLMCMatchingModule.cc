/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMCmatching/ECLMCMatchingModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLSim.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>


//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair



using namespace std;
using namespace boost;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLMCMatching)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLMCMatchingModule::ECLMCMatchingModule() : Module()
{
  // Set description
//  setDescription("ECLSimHitMakerModule");

  setDescription("ECLMCMatchingModule");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

}

ECLMCMatchingModule::~ECLMCMatchingModule()
{
}

void ECLMCMatchingModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  RelationArray::registerPersistent<ECLSimHit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLDigit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLShower, MCParticle>("", "");



}

void ECLMCMatchingModule::beginRun()
{
}

void ECLMCMatchingModule::event()
{


  int HitIndex[8736][16];
  int HitOldTrack[8736][16];
  int DigiIndex[8736];
  int DigiOldTrack[8736];
  int ShowerOldTrack[8736];
  for (int i = 0; i < 8736; i++) {
    DigiIndex[i] = -1; DigiOldTrack[i] = -1; ShowerOldTrack[i] = -1;
    for (int j = 0; j < 16; j++) {  HitIndex[i][j] = -1; HitOldTrack[i][j] = -1;}
  }

  StoreArray<MCParticle> mcParticles;
  PrimaryTrackMap eclPrimaryMap;
  eclPrimaryMap.clear();
  int nMcParticles = mcParticles.getEntries();
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    /*
       if(mcParticles[iPart]->getMother()==NULL)
                cout<<" "<<m_nEvent<<"Track "<<mcParticles[iPart]->getArrayIndex()
                <<" PDG "<<mcParticles[iPart]->getPDG()
                <<" P "<<mcParticles[iPart]->getMomentum().Mag()
                <<" theta "<< mcParticles[iPart]->getMomentum().Theta()* 180 /M_PI
                <<" phi "<<   mcParticles[iPart]->getMomentum().Phi()* 180 /M_PI
                <<" vx "<< mcParticles[iPart]->getProductionVertex().Perp()<<" "<<mcParticles[iPart]->getProductionVertex().z()<<endl;
         else
                cout<<" "<<m_nEvent<<"Track "<<mcParticles[iPart]->getArrayIndex()
                <<" PDG "<<mcParticles[iPart]->getPDG()
                <<" P "<<mcParticles[iPart]->getMomentum().Mag()
                <<" vx "<< mcParticles[iPart]->getProductionVertex().Perp()<<" "<<mcParticles[iPart]->getProductionVertex().z()
                <<" Mother "<< mcParticles[iPart]->getMother()->getArrayIndex()
                <<" PDG "<<mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getPDG()
                <<" P "<<mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getMomentum().Mag()<<endl;
     */
    if (mcParticles[iPart]->getMother() == NULL) {

      if (mcParticles[iPart]->getArrayIndex() == -1)
      {     eclPrimaryMap.insert(pair<int, int>(iPart, iPart));}
      else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
      //cout<<"mom "<<mcParticles[iPart]->getArrayIndex() <<endl;
    } else {
      eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex() ]));
      //cout<<"mom "<<mcParticles[iPart]->getMother()->getArrayIndex() <<" "<<mcParticles[iPart]->getArrayIndex()<<endl;
    }
  }

  StoreArray<ECLSimHit> eclSimHitArray;
  RelationArray eclSimHitRel(mcParticles, eclSimHitArray);
  StoreArray<ECLDigit> eclDigiArray;
  RelationArray  eclDigiToMCPart(eclDigiArray, mcParticles);

  int hitNum1 = eclDigiArray->GetEntriesFast();
  for (int ii = 0; ii < hitNum1; ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    int cId          = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}
    DigiIndex[cId] = ii;
  }


  for (int index = 0; index < eclSimHitRel.getEntries(); index++) {

    //MCParticle* aMCParticle = mcParticles[eclSimHitRel[index].getFromIndex()];
    int PrimaryIndex = -1;
    map<int, int>::iterator iter = eclPrimaryMap.find(eclSimHitRel[index].getFromIndex());
    if (iter != eclPrimaryMap.end()) {
      PrimaryIndex = iter->second;
    } else cout << "CantFind Track in eclPrimaryMap " << eclSimHitRel[index].getFromIndex() << endl;

    for (int hit = 0; hit < (int)eclSimHitRel[index].getToIndices().size(); hit++) {
      ECLSimHit* aECLSimHit = eclSimHitArray[eclSimHitRel[index].getToIndex(hit)];
      int hitCellId         = aECLSimHit->getCellId() - 1;
      if (DigiIndex[hitCellId] != -1 && DigiOldTrack[hitCellId] != PrimaryIndex) {
        eclDigiToMCPart.add(DigiIndex[hitCellId], PrimaryIndex);
        DigiOldTrack[hitCellId] = PrimaryIndex;
      }
    }//for (int hit = 0
  }//for index


  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  RelationArray  eclShowerToMCPart(eclRecShowerArray, mcParticles);


  MultiMap eclShowerMap;
  eclShowerMap.clear();

  const int ShowerNum = eclRecShowerArray->GetEntriesFast();
  const int hANum = eclHitAssignmentArray->GetEntriesFast();
  for (int iShower = 0; iShower < ShowerNum; iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->GetShowerId();
    for (int iHA = 0; iHA < hANum; iHA++) {
      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId();
      if (m_HAShowerId != showerId)continue;
      if (m_HAShowerId > showerId)break;
      eclShowerMap.insert(pair<int, int>(m_HAcellId, m_HAShowerId));
    }//for HA hANum
  }//ShowerNum

  MultiMap ShowerOldTrackMap;
  ShowerOldTrackMap.clear();

  //cout<<"Total showers in Event "<<m_nEvent<<" : "<< ShowerNum<<endl;
  for (int index = 0; index < eclDigiToMCPart.getEntries(); index++) {
    ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
    int cId          = (aECLDigi->getCellId() - 1);
    for (int iMCpart = 0; iMCpart < (int)eclDigiToMCPart[index].getToIndices().size(); iMCpart++) {

      int ShowerIndex = -1;
      MultiMap::size_type entries = eclShowerMap.count(cId);
      MultiMap::iterator iter = eclShowerMap.find(cId);
      for (MultiMap::size_type cnt = 0; cnt != entries; ++cnt) {

        ShowerIndex =  iter++->second ;
        bool OldRel = 0;
        MultiMap::size_type OldTrackEntries = ShowerOldTrackMap.count(ShowerIndex);
        MultiMap::iterator OldTrackIter = ShowerOldTrackMap.find(ShowerIndex);
        for (MultiMap::size_type iOldTrack = 0; iOldTrack != OldTrackEntries ; ++iOldTrack) {
          if (OldTrackIter++->second == (int) eclDigiToMCPart[index].getToIndex(iMCpart)) {
            OldRel = 1;
          }
        }



        if (ShowerIndex != -1 && OldRel == 0) {
          eclShowerToMCPart.add(ShowerIndex, eclDigiToMCPart[index].getToIndex(iMCpart));

          //ECLShower* aECLShower = eclRecShowerArray[ShowerIndex];
          //cout << "ShowerRel" << m_nEvent << " " << cId << " shower" << ShowerIndex
          //      << " Energy "<<  aECLShower->GetEnergy()
          //      << " theta "<<  aECLShower->GetTheta()* 180 /M_PI <<" phi "<<  aECLShower->GetPhi()* 180 /M_PI
          //      << " mom" << eclDigiToMCPart[index].getToIndex(iMCpart)
          //      << " PDG " << mcParticles[eclDigiToMCPart[index].getToIndex(iMCpart)]->getPDG()
          //      << endl;
          ShowerOldTrackMap.insert(pair<int, int>(ShowerIndex, eclDigiToMCPart[index].getToIndex(iMCpart)));

        }//ShowerIndex != -1
      }
    }//for iMCpart
  }//for index
  m_nEvent++;

}


void ECLMCMatchingModule::endRun()
{
  m_nRun++;
}

void ECLMCMatchingModule::terminate()
{
}


