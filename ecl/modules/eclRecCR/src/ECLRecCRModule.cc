/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecCR/ECLRecCRModule.h>
#include <ecl/dataobjects/DigiECL.h>
#include <ecl/dataobjects/RecCRECL.h>
#include <ecl/dataobjects/HitAssignmentECL.h>


#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCF.h>


#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include<ctime>
#include <iomanip>

// ROOT
#include <TVector3.h>

#define PI 3.14159265358979323846

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLRecCR)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLRecCRModule::ECLRecCRModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLRecCRHits from ECLDigi.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition
  addParam("ECLRecInput", m_eclDigiCollectionName,
           "Input Array // Output from ECLRecCR module or Data", string("ECLDigiHits"));

  //output
  addParam("ECLRecCROutput", m_eclRecCRName,
           "//Output of this module//(EventNo,CRId,cellId)", string("ECLRecCRHits"));


  addParam("ECLHitAssignmentinput", m_eclHitAssignmentName,
           "//input of this module//(EventNo,CRId,cellId)", string("ECLHitAssignment"));


//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));

}


ECLRecCRModule::~ECLRecCRModule()
{

}

void ECLRecCRModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void ECLRecCRModule::beginRun()
{
  B2INFO("ECLRecCRModule: Processing run: " << m_nRun);
}


void ECLRecCRModule::event()
{
  //Input Array
  StoreArray<DigiECL> eclDigiArray(m_eclDigiCollectionName);
  if (!eclDigiArray) {
    B2ERROR("Can not find ECLRecCRHits" << m_eclDigiCollectionName << ".");
  }


  int checkflag = 0;
  cout.unsetf(ios::scientific);
  cout.precision(6);
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();
  int hitNum = eclDigiArray->GetEntriesFast();
  TEclEnergyHit ss;
  int    get_ID = 0;
  for (int ii = 0; ii < hitNum; ii++) {
    DigiECL* aECLHit = eclDigiArray[ii];
    get_ID       =  aECLHit->getEventId();
    float FitEnergy    = (aECLHit->getAmp()) / 20000;//ADC count to GeV
//    double FitTime    = (1520 - aECLHit->getTimeFit()) * 24.*12 / 508 / (3072 / 2) ;//ADC count to us
    int cId          =  aECLHit->getCellId();
    if (FitEnergy < 0.) {continue;}

    cf.Accumulate(get_ID , FitEnergy, cId);
  }

  cf.SearchCRs();
  cf.Attributes();

  int nShower = 0;

//  cout<<"cf.CRs() "<<cf.CRs().size()<<endl;
  for (std::vector<TEclCFCR>::const_iterator iCR = cf.CRs().begin();
       iCR != cf.CRs().end(); ++iCR) {
//      cout<<"CR Energy: "<<iCR->Energy()<<" Mass:" <<iCR->Mass()<<" Width:"<<iCR->Width()<<" nshower  "<<iCR->Showers().size()<<endl;
    /// it should be gauranteed that the shower_id is in order in each cr


    for (EclCFShowerMap::const_iterator iShower = iCR->Showers().begin(); iShower != iCR->Showers().end(); ++iShower) {
      ///
      /// Shower Attributes
      ///
//      cout << "Event " << get_ID << " " << nShower << "shower Energy: " << (*iShower).second.Energy() << " Theta " << (*iShower).second.Theta() / PI * 180 << " Phi " << (*iShower).second.Phi() / PI * 180 << " " << iCR->Showers().size() << endl;

      TEclCFShower iSh = (*iShower).second;
      std::vector<MEclCFShowerHA> HAs = iSh.HitAssignment();
      for (std::vector<MEclCFShowerHA>::iterator iHA = HAs.begin();
           iHA != HAs.end(); ++iHA) {

        StoreArray<HitAssignmentECL> eclHaArray(m_eclHitAssignmentName);
        m_HANum = eclHaArray->GetLast() + 1;
        new(eclHaArray->AddrAt(m_HANum)) HitAssignmentECL();
        eclHaArray[m_HANum]->setShowerId(nShower);
        eclHaArray[m_HANum]->setCellId(iHA->Id());

//        cout<<iHA->Id()<<" ";
      }
//        cout<<endl;
      StoreArray<RecCRECL> eclRecCRArray(m_eclRecCRName);
      m_hitNum = eclRecCRArray->GetLast() + 1;
      new(eclRecCRArray->AddrAt(m_hitNum)) RecCRECL();
      eclRecCRArray[m_hitNum]->setShowerId(nShower);
      eclRecCRArray[m_hitNum]->setEnergy((*iShower).second.Energy());
      eclRecCRArray[m_hitNum]->setTheta((*iShower).second.Theta());
      eclRecCRArray[m_hitNum]->setPhi((*iShower).second.Phi());
      eclRecCRArray[m_hitNum]->setDistance((*iShower).second.Distance());
      eclRecCRArray[m_hitNum]->setMass((*iShower).second.Mass());
      eclRecCRArray[m_hitNum]->setWidth((*iShower).second.Width());
      eclRecCRArray[m_hitNum]->setE9oE25((*iShower).second.E9oE25());
      eclRecCRArray[m_hitNum]->setTotEnergy((*iShower).second.TotEnergy());
      eclRecCRArray[m_hitNum]->setE9oE25unf((*iShower).second.E9oE25unf());
      eclRecCRArray[m_hitNum]->setUncEnergy((*iShower).second.UncEnergy());
      eclRecCRArray[m_hitNum]->setNHits((*iShower).second.NHits());
      eclRecCRArray[m_hitNum]->setWNHits((*iShower).second.WNHits());
      eclRecCRArray[m_hitNum]->setNHitsUsed((*iShower).second.NHitsUsed());
      eclRecCRArray[m_hitNum]->setStatus((*iShower).second.Status());
      eclRecCRArray[m_hitNum]->setGrade((*iShower).second.Grade());
      nShower++;
      if (((*iShower).second.Energy() > 0.05 && (*iShower).second.Energy() < 0.2)) {
        checkflag = 1;
      }



    }//EclCFShowerMap
  }//vector<TEclCFCR>

  /*
    if (checkflag) {
      for (std::vector<TEclCFCR>::const_iterator iCR = cf.CRs().begin();
           iCR != cf.CRs().end(); ++iCR) {
        for (EclCFShowerMap::const_iterator iShower = iCR->Showers().begin(); iShower != iCR->Showers().end(); ++iShower) {
          cout << "Event " << get_ID << " SHOWER " << (*iShower).second.Energy() << " " << (*iShower).second.Theta() / PI * 180 << " " << (*iShower).second.Phi() / PI * 180 << " NHits " << (*iShower).second.NHits() << endl;
        }
        for (EclEnergyHitMap::const_iterator iSeed = iCR->Seeds().begin(); iSeed != iCR->Seeds().end(); ++iSeed) {
          TEclEnergyHit iSe = (*iSeed).second;
  //         cout<<"Seed "<<iSe.CellId()<<" "<<iSe.Energy()<<endl;
        }

      }//vector<TEclCFCR>
      for (int ia = 0; ia < hitNum; ia++) {
        DigiECL* aECLHit = eclDigiArray[ia];
        float FitEnergy    = (aECLHit->getAmp()) / 20000;//ADC count to GeV
        int cId          =  aECLHit->getCellId();
        ECLGeometryPar* eclp = ECLGeometryPar::Instance();
        TVector3 PosCell =  eclp->GetCrystalPos(cId);
        eclp->Mapping(cId);
        cout << "HitHA " << cId << " " << eclp->GetThetaID() << " " << eclp-> GetPhiID() << " " << FitEnergy << " "
             << PosCell.Theta() / PI * 180 << " " << PosCell.Phi() / PI * 180 << endl;
      }
    }

  */
  m_nEvent++;

}

void ECLRecCRModule::endRun()
{
  m_nRun++;
}

void ECLRecCRModule::terminate()
{
  m_timeCPU = clock() * Unit::us - m_timeCPU;

}


