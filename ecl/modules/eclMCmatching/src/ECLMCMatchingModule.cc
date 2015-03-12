/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Benjamin Oberhof                            *
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

#include <mdst/dataobjects/ECLCluster.h>

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

  setDescription("ECLMCMatchingModule");
  setPropertyFlags(c_ParallelProcessingCertified);

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
  StoreArray<MCParticle> MCParticleArray;
  StoreArray<ECLHit> ECLHitArray;
  StoreArray<ECLDigit> ECLDigitArray;
  StoreArray<ECLShower> ECLShowerArray;
  StoreArray<ECLCluster> ECLClusterArray;
  MCParticleArray.registerRelationTo(ECLHitArray);
  //MCParticleArray.registerRelationTo(ECLDigitArray);
  ECLDigitArray.registerRelationTo(MCParticleArray);
  ECLShowerArray.registerRelationTo(MCParticleArray);
  ECLClusterArray.registerRelationTo(MCParticleArray);

  StoreArray<ECLSimHit> ECLSimHitArray;
  ECLSimHitArray.registerRelationTo(ECLHitArray);

  // RelationArray::registerPersistent<ECLHit,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLDigit,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLShower,MCParticle>("", ""); obsolete
  // RelationArray::registerPersistent<ECLCluster,MCParticle>("", ""); obsolete

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
  if (nParticles > 1000)nParticles = 1000; //skip mcParticles from to speed up

  for (int iPart = 0; iPart < nParticles ; iPart++) {
    if (mcParticles[iPart]->getMother() == NULL
        && !mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && !mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator)) continue;

    bool adhoc_StableInGeneratorFlag(mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator));
    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle) && adhoc_StableInGeneratorFlag) {
      if (mcParticles[iPart]->getArrayIndex() == -1) {
        eclPrimaryMap.insert(pair<int, int>(iPart, iPart));
      } else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
    } else {
      if (mcParticles[iPart]->getMother() == NULL) continue;
      if (eclPrimaryMap.find(mcParticles[iPart]->getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
        eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex()]));
      }//if mother of mcParticles is stored.
    }//if c_StableInGenerator and c_PrimaryParticle
  }//for mcParticles


  StoreArray<ECLHit> eclHitArray;
  RelationArray eclHitRel(mcParticles, eclHitArray);
  StoreArray<ECLDigit> eclDigiArray;
  RelationArray eclDigiToMCPart(eclDigiArray, mcParticles);
  StoreArray<ECLSimHit> eclSimHitArray;
  RelationArray eclHitToSimHit(eclHitArray, eclSimHitArray);
  RelationArray eclSimHitRel(mcParticles, eclSimHitArray);

  for (int ii = 0; ii < eclDigiArray.getEntries(); ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.; //ADC count to GeV
    int cId            = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {
      continue;
    }
    DigiIndex[cId] = ii;
  }

  for (int index = 0; index < eclHitRel.getEntries(); index++) {
    int PrimaryIndex = -1;
    map<int, int>::iterator iter = eclPrimaryMap.find(eclHitRel[index].getFromIndex());
    if (iter != eclPrimaryMap.end()) {
      PrimaryIndex = iter->first; //it's the daughter
    } else continue;

    for (int hit = 0; hit < (int)eclHitRel[index].getToIndices().size(); hit++) {
      ECLHit* aECLHit = eclHitArray[eclHitRel[index].getToIndex(hit)];
      int hitCellId         = aECLHit->getCellId() - 1;
      if (aECLHit->getBackgroundTag() != 0) continue; //is it necessary with previous condition on primary map?

      if (DigiIndex[hitCellId] != -1 && DigiOldTrack[hitCellId] != PrimaryIndex) {
        eclDigiToMCPart.add(DigiIndex[hitCellId], PrimaryIndex);
        DigiOldTrack[hitCellId] = PrimaryIndex;
      }
    }//for (int hit = 0
  }//for index


  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  RelationArray  eclShowerToMCPart(eclRecShowerArray, mcParticles);
  RelationArray  eclShowerToMCParts(eclRecShowerArray, mcParticles);
  map<int, float> mc_relations;

  //PrimaryTrackMap eclMCParticleContributionMap; //the cell could be below to several
  map<int, float> eclMCParticleContributionMap;;
  eclMCParticleContributionMap.clear();
  mc_relations.clear();

  //unsigned int jj=0;
  //unsigned int kk=0;
  for (int iShower = 0; iShower < eclRecShowerArray.getEntries(); iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->getShowerId();
    //float ene=0.;
    for (int iMCPart = 0; iMCPart < eclSimHitRel.getEntries(); iMCPart++) {
      float ene = 0;
      for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
        //cout << "^^^^^^^^^^^^^^^" << eclHitAssignmentArray.getEntries() << "**************" << eclHitArray.getEntries() << "&&&&&&&&&&&&&&&&&&&&" << eclHitRel.getEntries() << endl;
        ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
        int m_HAShowerId = aECLHitAssignment->getShowerId();
        int m_HAcellId = aECLHitAssignment->getCellId() - 1 ;
        if (m_HAShowerId != showerId)continue;
        if (m_HAShowerId > showerId)break;

        //      float ene=0.;
        //      for (int iMCPart = 0; iMCPart < eclSimHitRel.getEntries(); iMCPart++) {
        for (int simhit = 0; simhit < (int)eclSimHitRel[iMCPart].getToIndices().size(); simhit++) { //pleonastico
          //for (int simhit = 0; simhit < (int)eclHitRel[iMCPart].getToIndices().size(); simhit++)
          ECLSimHit* aECLSimHit = eclSimHitArray[eclSimHitRel[iMCPart].getToIndex(simhit)];
          int hitCellId         = aECLSimHit->getCellId() - 1;
          //double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
          if ((hitCellId != m_HAcellId)) continue;

          map<int, float>::iterator iter =  eclMCParticleContributionMap.find((int) eclSimHitRel[iMCPart].getToIndex(simhit));
          if (iter == eclMCParticleContributionMap.end()) {
            ene = ene + aECLSimHit->getEnergyDep();
            //iter->second += ene;
            //eclMCParticleContributionMap.insert(pair<int, float>((int) eclSimHitRel[iMCPart].getFromIndex(), ene));
            cout << "End, ShowerId: " << showerId << " MCParticle: " << iMCPart << " Energy dep: " << aECLSimHit->getEnergyDep() << " Ene: " << ene << " Crystal: " << aECLSimHit->getCellId() - 1 << endl;
            cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
          } else {
            ene = ene + aECLSimHit->getEnergyDep();
            //iter->second += ene;
            //iter->second += aECLSimHit->getEnergyDep();
            cout << "ShowerId: " << showerId << " MCParticle: " << iMCPart << " Energy dep: " << aECLSimHit->getEnergyDep() << " Crystal: " << aECLSimHit->getCellId() - 1 << endl;
            cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
          }
          cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
        }//for simhit
        //cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
      }//for hANum //iMCPart
      if (ene > 0.) {
        eclMCParticleContributionMap.insert(pair<int, float>((int) eclSimHitRel[iMCPart].getFromIndex(), ene));
      }
    }//for iMCPart //HA hANum

    int PrimaryIndex = -1;
    int MaxContribution = 0;
    //cout << "Contributions to shower " << showerId << " : " << eclMCParticleContributionMap.size() << endl;
    for (map<int, float>::iterator i = eclMCParticleContributionMap.begin(); i != eclMCParticleContributionMap.end(); ++i) {
      cout << "ShowerID: " << showerId << " Energy: " << aECLShower->getEnergy() << " MCParticle: " << (*i).first << endl;
      cout << "Cand.: " << (*i).first << " contribution (GeV): " << (*i).second << endl;
      cout << "********************************************" << endl;
      mc_relations[(*i).first] += (*i).second;///aECLShower->getEnergy();
      //      eclShowerToMCParts.add(showerId,  mc_relations.begin(), mc_relations.end());
      if ((*i).second > MaxContribution) {MaxContribution = (*i).second ;  PrimaryIndex = (*i).first ;}
    }

    for (map<int, float>::iterator i = mc_relations.begin(); i != mc_relations.end(); ++i) {
      //if(mc_relations.size()>kk){
      cout << "ShowerID: " << showerId << " MCParticle: " << (*i).first << endl;
      cout << "Check cand.: " << (*i).first << " contribution (GeV): " << (*i).second << endl;
      cout << "-----------------------------------------" << endl;
      //mc_relations[(*i).first] += (*i).second;
      //      eclShowerToMCParts.add(showerId,  mc_relations.begin(), mc_relations.end());
      //kk++;
      //}
    }
    cout << "-----------------------------------------" << endl;
    //*/
    eclMCParticleContributionMap.clear();

    if (PrimaryIndex == -1) {
      //Shower is not due to primary particles but is supposed due to BeamBG particles
      //need more BeamBG  sample to test if CPU time consuming is acceptable
      /*
      double MaxEnergy = 0;
      for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId() - 1;
      ECLShower* aECLShower = eclRecShowerArray[m_HAShowerId];
      float ShowerEnergy = (aECLShower ->getEnergy()) / 20000.;
      if (m_HAShowerId != showerId)continue;
      if (m_HAShowerId > showerId)break;

      for (int iMCPart = 0; iMCPart < eclHitRel.getEntries(); iMCPart++) {
      for (int hit = 0; hit < (int)eclHitRel[iMCPart].getToIndices().size(); hit++) {
      //ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
      //ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
      //float ShowerEnergy    = (aECLDigi->getAmp()) / 20000.;
      ECLHit* aECLHit = eclHitArray[eclHitRel[iMCPart].getToIndex(hit)];
      int hitCellId         = aECLHit->getCellId() - 1;
      double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
      if (hitCellId==m_HAcellId  &&  hitE>MaxEnergy) {
      MaxEnergy=hitE;
      if(MaxEnergy>0.8*ShowerEnergy) {
      PrimaryIndex =eclHitRel[iMCPart].getFromIndex();
      }//if condition
      }
      }//for  hit
      }//for iMCPart
      }//for HA hANum
      eclShowerToMCPart.add(showerId, PrimaryIndex);
      //
      */
    } else {
      /*
      double MaxEnergy = 0;
      for (int iHA = 0; iHA <  eclHitAssignmentArray.getEntries(); iHA++) {
      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId() - 1;
      ECLShower* aECLShower = eclRecShowerArray[m_HAShowerId];
      float ShowerEnergy = (aECLShower ->getEnergy());
        if (m_HAShowerId != showerId)continue;
        if (m_HAShowerId > showerId)break;

        for (int iMCPart = 0; iMCPart < eclHitRel.getEntries(); iMCPart++) {
      for (int hit = 0; hit < (int)eclHitRel[iMCPart].getToIndices().size(); hit++) {
      //ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
      //ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
      //float ShowerEnergy    = (aECLDigi->getAmp()) / 20000.;
      ECLHit* aECLHit = eclHitArray[eclHitRel[iMCPart].getToIndex(hit)];
      int hitCellId         = aECLHit->getCellId() - 1;
      double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
      if (hitCellId==m_HAcellId  &&  hitE>MaxEnergy) {
      MaxEnergy=hitE;
      if(MaxEnergy>0.25*ShowerEnergy) {
      eclShowerToMCPart.add(showerId, PrimaryIndex);
      cout << "Max: " << MaxEnergy << " Shower: " << ShowerEnergy << endl;
      //PrimaryIndex =eclHitRel[iMCPart].getFromIndex();
      }//if condition
      }
      }//for  hit
      }//for iMCPart
      }//for HA hANum
      */
      //eclShowerToMCPart.add(showerId, PrimaryIndex);

      for (map<int, float>::iterator i = mc_relations.begin(); i != mc_relations.end(); ++i) {
        cout << "MC Relations: " << mc_relations.size() << endl;
        //if((mc_relations.size())>(jj)){
        cout << "ShowerID: " << showerId << " MCParticle: " << (*i).first << endl;
        eclShowerToMCPart.add(showerId, (*i).first, (*i).second);
        cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& " << endl;
        //jj++;
        //}
      }
      cout << "Winner, ShowerID: " << showerId << " MCParticle: " << PrimaryIndex << endl;
      cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& " << endl;
    }
    mc_relations.clear();
  }//ShowerNum

  //Following FOR is for test purposes
  for (int index = 0; index < eclShowerToMCPart.getEntries(); index++) {
    cout << "Entries: " << eclShowerToMCPart.getEntries() << endl;
    cout << "ShowerId: " << (int)eclShowerToMCPart[index].getFromIndex() << endl;
    cout << "Cand.: " << (int)eclShowerToMCPart[index].getToIndex() << endl;
    cout << "Weight: " << (float)eclShowerToMCPart[index].getWeight() << endl;
    cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
  }
  cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;


  //... Related ECLClustertoMCParticle
  //... First get relation of ECLCluster to ECLShower
  //... Then exploit already available ECLShower relation to MCParticle
  //... get the index of MCParticle from the relation of ECLShower
  //... and use it to relate ECLCluster to MCParticle
  //... ECLCLuster --> ECLShower --> getShowerId()
  //... ECLShower(ShowerId() --> MCParticle --> getIndex();
  //... ECLCluster -> related to --> MCParticle
  //... Vishal

  StoreArray<ECLCluster> eclClusterArray;
  RelationArray ECLClustertoMCPart(eclClusterArray, mcParticles);


  for (int imdst = 0; imdst < eclClusterArray.getEntries(); ++imdst) {
    const ECLCluster* cluster = eclClusterArray[imdst];
    for (auto eclShower : cluster->getRelationsTo<ECLShower>()) {
      ECLShower* veclShower = eclRecShowerArray[eclShower.getShowerId()];
      //const ECLShower *shower = ecl
      for (auto MCpart : veclShower->getRelationsTo<MCParticle>()) {
        ECLClustertoMCPart.add(imdst, MCpart.getArrayIndex());
      }
    }
  } // imdst

  //.. End here ECLtoMCparticle relation

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
  cout << "Event: " << m_nEvent << endl;
  cout << "-----------------------------------------" << endl;
  cout << "-----------------------------------------" << endl;
  m_nEvent++;

}


void ECLMCMatchingModule::endRun()
{
  m_nRun++;
}

void ECLMCMatchingModule::terminate()
{
}


