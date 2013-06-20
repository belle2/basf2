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
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>


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
//  setDescription("ECLHitMakerModule");

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
  RelationArray::registerPersistent<ECLHit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLDigit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLShower, MCParticle>("", "");



}

void ECLMCMatchingModule::beginRun()
{
}

void ECLMCMatchingModule::event()
{


  int DigiIndex[8736];
  int DigiOldTrack[8736];
  for (int i = 0; i < 8736; i++) {
    DigiIndex[i] = -1; DigiOldTrack[i] = -1;
  }

  StoreArray<MCParticle> mcParticles;
  PrimaryTrackMap eclPrimaryMap;
  eclPrimaryMap.clear();
  int nMcParticles = mcParticles.getEntries();
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    /*
                if (mcParticles[iPart]->getMother() == NULL)
                  cout << "Event " << m_nEvent << " primary track ID " << mcParticles[iPart]->getArrayIndex()
                       << " pdg " << mcParticles[iPart]->getPDG()
                       << " p " << mcParticles[iPart]->getMomentum().Mag()
                       << " theta " << mcParticles[iPart]->getMomentum().Theta() * 180 / M_PI
                       << " phi " <<   mcParticles[iPart]->getMomentum().Phi() * 180 / M_PI
                       << " vertex R & Z " << mcParticles[iPart]->getProductionVertex().Perp() << " " << mcParticles[iPart]->getProductionVertex().z() << endl;
                else
                  cout << "Event " << m_nEvent << "Track " << mcParticles[iPart]->getArrayIndex()
                       << " PDG " << mcParticles[iPart]->getPDG()
                       << " P " << mcParticles[iPart]->getMomentum().Mag()
                       << " vx " << mcParticles[iPart]->getProductionVertex().Perp() << " " << mcParticles[iPart]->getProductionVertex().z()
                       << " Mother " << mcParticles[iPart]->getMother()->getArrayIndex()
                       << " PDG " << mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getPDG()
                       << " P " << mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getMomentum().Mag()
                       << " c_StableInGenerator "<<mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator)
                       << " c_PrimaryParticle " <<mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle) << endl;
    */
    bool adhoc_StableInGeneratorFlag(mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator));


    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle) && ! adhoc_StableInGeneratorFlag
        && mcParticles[iPart]->getFirstDaughter() > 0) {
      for (int daughterIndex = mcParticles[iPart]->getFirstDaughter() - 1;
           daughterIndex < mcParticles[iPart]->getLastDaughter();
           daughterIndex++) {
        if (! mcParticles[daughterIndex]->hasStatus(MCParticle::c_PrimaryParticle)) {
          adhoc_StableInGeneratorFlag = true;
        }
      }
    }



    // skip particles decayed in the generator
    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle) && !adhoc_StableInGeneratorFlag) continue;


    // fill primary particles as the origins
    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && adhoc_StableInGeneratorFlag) {

      if (mcParticles[iPart]->getArrayIndex() == -1)
      {     eclPrimaryMap.insert(pair<int, int>(iPart, iPart));}
      else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
    } else {
      if (eclPrimaryMap.find(mcParticles[iPart]->getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
        eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex()]));
      } else {
        //B2ERROR(boost::format("Cannot find eclPrimaryMap entry for the mother of MCParticle ID: %d")
        //        % mcParticles[iPart]->getMother()->getArrayIndex());
        eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));
      }
      //cout<<"mom "<<mcParticles[iPart]->getMother()->getArrayIndex() <<" "<<mcParticles[iPart]->getArrayIndex()<<endl;
    }


  }



  StoreArray<ECLHit> eclHitArray;
  RelationArray eclHitRel(mcParticles, eclHitArray);
  StoreArray<ECLDigit> eclDigiArray;
  RelationArray  eclDigiToMCPart(eclDigiArray, mcParticles);

  int hitNum1 = eclDigiArray.getEntries();
  for (int ii = 0; ii < hitNum1; ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    int cId          = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}
    DigiIndex[cId] = ii;
  }

  for (int index = 0; index < eclHitRel.getEntries(); index++) {

    //MCParticle* aMCParticle = mcParticles[eclHitRel[index].getFromIndex()];
    int PrimaryIndex = -1;
    map<int, int>::iterator iter = eclPrimaryMap.find(eclHitRel[index].getFromIndex());

    if (iter != eclPrimaryMap.end()) {
      PrimaryIndex = iter->second;
    } else continue;

    for (int hit = 0; hit < (int)eclHitRel[index].getToIndices().size(); hit++) {
      ECLHit* aECLHit = eclHitArray[eclHitRel[index].getToIndex(hit)];
      int hitCellId         = aECLHit->getCellId() - 1;
      if (aECLHit->getBackgroundTag() != 0) continue;

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

  const int ShowerNum = eclRecShowerArray.getEntries();//->GetEntriesFast();
  const int hANum = eclHitAssignmentArray.getEntries();//->GetEntriesFast();
  for (int iShower = 0; iShower < ShowerNum; iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->getShowerId();
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

      if (eclDigiToMCPart[index].getToIndex(iMCpart) > 1000)cout << " index " << index << " iMCpart  " << iMCpart << " PrimaryIndex  " << eclDigiToMCPart[index].getToIndex(iMCpart) << endl;
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
          /*
          ECLShower* aECLShower = eclRecShowerArray[ShowerIndex];
                     cout << "Event" << m_nEvent  << " RecShower" << ShowerIndex
                          << " Energy " <<  aECLShower->getEnergy()
                          << " theta " <<  aECLShower->getTheta() * 180 / M_PI << " phi " <<  aECLShower->getPhi() * 180 / M_PI
                          << " mom" << eclDigiToMCPart[index].getToIndex(iMCpart)
                          << " PDG " << mcParticles[eclDigiToMCPart[index].getToIndex(iMCpart)]->getPDG()
                          << endl;
          */
          ShowerOldTrackMap.insert(pair<int, int>(ShowerIndex, eclDigiToMCPart[index].getToIndex(iMCpart)));

        }//ShowerIndex != -1
      }//
    }//for iMCpart
  }//for index
  /*
    StoreArray<ECLGamma> gammaArray;
    RelationArray eclGammaToShower(gammaArray, eclRecShowerArray);
    if (gammaArray) {
      for (int iIndex = 0; iIndex < eclGammaToShower.getEntries() ; iIndex++) {
        for (int iHit = 0; iHit < (int)eclGammaToShower[iIndex].getToIndices().size(); iHit++) {
          for (int index = 0; index < eclShowerToMCPart.getEntries(); index++) {
            if (eclGammaToShower[iIndex].getToIndex(iHit) == eclShowerToMCPart[index].getFromIndex()) {
              for (int iMCpart = 0; iMCpart < (int)eclShowerToMCPart[index].getToIndices().size(); iMCpart++) {
                ECLShower* aECLShower = eclRecShowerArray[ eclGammaToShower[iIndex].getToIndex(iHit) ];
                cout << "Event " << m_nEvent << " Rec Gamma from shower" << eclShowerToMCPart[index].getFromIndex()
                     << " Energy " <<  aECLShower->getEnergy()
                     << " theta " <<  aECLShower->getTheta() * 180 / M_PI << " phi " <<  aECLShower->getPhi() * 180 / M_PI
                     << " From primary track" <<  eclShowerToMCPart[index].getToIndex(iMCpart)
                     << " PDG " << mcParticles[ eclShowerToMCPart[index].getToIndex(iMCpart)]->getPDG()
                     << endl;
              }//for all matched mcparticle
            }//if same shower ID
          }//for all eclShowerToMCPart relation
        }//for the shower to reconstruct gamma
      }//for all eclGammaToShower relation
    }//if gammaArray exit
  */
  /*
    StoreArray<ECLPi0> Pi0Array;
    StoreArray<ECLGamma> gammaArray;
    RelationArray eclPi0ToGamma(Pi0Array, gammaArray);
    if (Pi0Array) {
      for (int iPi0 = 0; iPi0 < Pi0Array.getEntries() ; iPi0++) {
        ECLPi0* aECLPi0 = Pi0Array[ iPi0 ];
        cout << "Pi0Event" << m_nEvent<<" iIndex  "<<iPi0 << " Energy " << aECLPi0->getEnergy() << " Mass " << aECLPi0->getMass()
             << " MassFit " << aECLPi0->getMassFit() << " Chi2 " << aECLPi0->getChi2()
             << " getPValue " << aECLPi0->getPValue() << endl;
        TMatrixFSym Pi0ErrorMatrix(7);
          aECLPi0-> getErrorMatrix7x7(Pi0ErrorMatrix);
          for (int i = 0; i < 7; i++) {
              for (int j = 0; j <= i ; j++) {
                cout<< Pi0ErrorMatrix[i][j]<<" ";
              }
                cout<<endl;
          }

      for (int iIndex = 0; iIndex < eclPi0ToGamma.getEntries() ; iIndex++) {
       if( (int)eclPi0ToGamma[iIndex].getFromIndex()!=iPi0 ) continue;
        for (int iHit = 0; iHit < (int)eclPi0ToGamma[iIndex].getToIndices().size(); iHit++) {

          ECLPi0* aECLPi0 = Pi0Array[ eclPi0ToGamma[iIndex].getFromIndex()];
          ECLGamma* aECLGamma = gammaArray[ eclPi0ToGamma[iIndex].getToIndex(iHit) ];
          cout << "Event" << m_nEvent << " Rec "<<eclPi0ToGamma[iIndex].getFromIndex()<<"Pi0 Energy "<<  aECLPi0->getEnergy()
               << " from Gamma " <<  aECLGamma->getShowerId()
               << " Energy " <<  aECLGamma->getEnergy()
               << " Px " <<  aECLGamma->getPx()
               << " Py " <<  aECLGamma->getPy()
               << " Pz " <<  aECLGamma->getPz()
               << endl;

          TMatrixFSym GammaErrorMatrix(7);
          aECLGamma-> getErrorMatrix7x7(GammaErrorMatrix);
          for (int i = 0; i < 7; i++) {
              for (int j = 0; j <= i ; j++) {
                cout<< GammaErrorMatrix[i][j]<<" ";
              }
                cout<<endl;
          }
      }//for iPi0
        }//for the shower to reconstruct gamma
      }//for all Pi0ToGamma relation
    }//if pi0Array exit
   */
  m_nEvent++;

}


void ECLMCMatchingModule::endRun()
{
  m_nRun++;
}

void ECLMCMatchingModule::terminate()
{
}


