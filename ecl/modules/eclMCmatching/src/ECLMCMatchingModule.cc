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


#include <mdst/dataobjects/MCParticle.h>
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
  // CPU time start
  m_timeCPU = clock() * Unit::us;
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
  int nParticles = mcParticles.getEntries();
  if (nParticles > 1000)nParticles = 1000; //skip many mcParticles from ROF to speed up
  for (int iPart = 0; iPart < nParticles ; ++iPart) {
    if (mcParticles[iPart]->getMother() == NULL
        && !mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && !mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator)) continue;

    bool adhoc_StableInGeneratorFlag(mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator));
    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && adhoc_StableInGeneratorFlag) {
      if (mcParticles[iPart]->getArrayIndex() == -1)
      {     eclPrimaryMap.insert(pair<int, int>(iPart, iPart));}
      else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
    } else {
      if (mcParticles[iPart]->getMother() == NULL) continue;
      if (eclPrimaryMap.find(mcParticles[iPart]->getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
        eclPrimaryMap.insert(
          pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex()]));
      }//if mother of mcParticles is stored.
    }//if c_StableInGenerator and c_PrimaryParticle

  }//for mcParticles


  StoreArray<ECLHit> eclHitArray;
  RelationArray eclHitRel(mcParticles, eclHitArray);
  StoreArray<ECLDigit> eclDigiArray;
  RelationArray  eclDigiToMCPart(eclDigiArray, mcParticles);

  for (int ii = 0; ii < eclDigiArray.getEntries(); ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    int cId          = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}
    DigiIndex[cId] = ii;
  }


  for (int index = 0; index < eclHitRel.getEntries(); index++) {

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

  PrimaryTrackMap eclMCParticleContributionMap;//the cell could be below to several
  eclMCParticleContributionMap.clear();


  for (int iShower = 0; iShower < eclRecShowerArray.getEntries(); iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->getShowerId();
    for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId() - 1 ;
      if (m_HAShowerId != showerId)continue;
      if (m_HAShowerId > showerId)break;

      for (int index = 0; index < eclDigiToMCPart.getEntries(); index++) {
        ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
        int cId          = (aECLDigi->getCellId() - 1);
        if (cId != m_HAcellId)continue;
        for (int iMCpart = 0; iMCpart < (int)eclDigiToMCPart[index].getToIndices().size(); iMCpart++) {
          map<int, int>::iterator iter =  eclMCParticleContributionMap.find((int) eclDigiToMCPart[index].getToIndex(iMCpart));
          if (iter == eclMCParticleContributionMap.end()) {
            eclMCParticleContributionMap.insert(pair<int, int>((int) eclDigiToMCPart[index].getToIndex(iMCpart), aECLDigi->getAmp()));
          } else {
            iter->second += aECLDigi->getAmp();
          }
        }//loop MCparticle with same aECLDigi
      }//loop aECLDigi to MCparticle
    }//for HA hANum

    int PrimaryIndex = -1;
    int MaxContribution = 0;
    for (map<int, int>::iterator i = eclMCParticleContributionMap.begin(); i != eclMCParticleContributionMap.end(); ++i) {
      if ((*i).second > MaxContribution) {MaxContribution = (*i).second ;  PrimaryIndex = (*i).first; }
    }

    eclMCParticleContributionMap.clear();

    if (PrimaryIndex == -1) {//Shower is not due to primary particles whcih is supposed due to BeamBG particles
      /*//need more BeamBG  sample to test if CPU time consuming is acceptable

      double MaxEnergy = 0;
      for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
        ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
        int m_HAShowerId = aECLHitAssignment->getShowerId();
        int m_HAcellId = aECLHitAssignment->getCellId() - 1 ;
        if (m_HAShowerId != showerId)continue;
        if (m_HAShowerId > showerId)break;

        for (int iMCPart = 0; iMCPart < eclHitRel.getEntries(); iMCPart++) {
          for (int hit = 0; hit < (int)eclHitRel[iMCPart].getToIndices().size(); hit++) {
            ECLHit* aECLHit = eclHitArray[eclHitRel[iMCPart].getToIndex(hit)];
            int hitCellId         = aECLHit->getCellId() - 1;
            double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
            if (hitCellId==m_HAcellId  &&hitE> MaxEnergy ) {
              MaxEnergy=hitE;
              PrimaryIndex =eclHitRel[iMCPart].getFromIndex();

            }
          }//for  hit
        }//for iMCPart
      }//for HA hANum
      eclShowerToMCPart.add(showerId, PrimaryIndex);
      *///need more BeamBG  sample to test if CPU time consuming is acceptable
    } else {
      eclShowerToMCPart.add(showerId, PrimaryIndex);
    }
  }//ShowerNum

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


