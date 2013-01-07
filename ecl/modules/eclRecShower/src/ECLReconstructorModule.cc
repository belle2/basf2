/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecShower/ECLReconstructorModule.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>


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
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLReconstructorModule::ECLReconstructorModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLRecShower from ECLDigi.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}


ECLReconstructorModule::~ECLReconstructorModule()
{

}

void ECLReconstructorModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
  StoreArray<ECLHitAssignment>::registerPersistent();
  StoreArray<ECLShower>::registerPersistent();


}

void ECLReconstructorModule::beginRun()
{
  B2INFO("ECLReconstructorModule: Processing run: " << m_nRun);
}


void ECLReconstructorModule::event()
{
  //Input Array
  StoreArray<ECLDigit> eclDigiArray;
  if (!eclDigiArray) {
    B2DEBUG(100, "ECLDigit in empty in event " << m_nEvent);
    return;
  }

//  int checkflag = 0;
  cout.unsetf(ios::scientific);
  cout.precision(6);
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();

  //cout<<"Event "<< m_nEvent<<" Total input entries of Digi Array  "<<eclDigiArray->GetEntriesFast()<<endl;
  int hitNum = eclDigiArray->GetEntriesFast();
  TEclEnergyHit ss;
  for (int ii = 0; ii < hitNum; ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV

    int cId          = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}

    cf.Accumulate(m_nEvent, FitEnergy, cId);
  }

  cf.SearchCRs();
  cf.Attributes();

  int nShower = 0;

  for (std::vector<TEclCFCR>::const_iterator iCR = cf.CRs().begin();
       iCR != cf.CRs().end(); ++iCR) {


    for (EclCFShowerMap::const_iterator iShower = iCR->Showers().begin(); iShower != iCR->Showers().end(); ++iShower) {
      ///
      /// Shower Attributes
      ///
//      cout << "shower  " << nShower << "shower Energy: " << (*iShower).second.Energy() << " Theta " << (*iShower).second.Theta() / PI * 180 << " Phi " << (*iShower).second.Phi() / PI * 180 << " " <<  (float)(*iShower).second.E9oE25()<< endl;

      TEclCFShower iSh = (*iShower).second;
      std::vector<MEclCFShowerHA> HAs = iSh.HitAssignment();
      for (std::vector<MEclCFShowerHA>::iterator iHA = HAs.begin();
           iHA != HAs.end(); ++iHA) {

        StoreArray<ECLHitAssignment> eclHaArray;
        if (!eclHaArray) eclHaArray.create();
        m_HANum = eclHaArray->GetLast() + 1;
        new(eclHaArray->AddrAt(m_HANum)) ECLHitAssignment();
        eclHaArray[m_HANum]->setShowerId(nShower);
        eclHaArray[m_HANum]->setCellId(iHA->Id());

//        cout<<iHA->Id()<<" ";
      }
//        cout<<endl;

      double energyBfCorrect = (*iShower).second.Energy();
      double preliminaryCalibration = 1.0;
      if ((*iShower).second.Theta() > 0 && ((*iShower).second.Theta() / PI * 180) < 33.2) {
        preliminaryCalibration = 0.95960 - 0.4953E-02 * pow(log10(energyBfCorrect), 1)
                                 - 0.1463E-01 * pow(log10(energyBfCorrect), 2)
                                 + 0.4869E-03 * pow(log10(energyBfCorrect), 3);
      } else if (((*iShower).second.Theta() / PI * 180) < 126.1) {
        preliminaryCalibration = 0.9650 - 0.1090E-01 * pow(log10(energyBfCorrect), 1)
                                 - 0.6164E-02 * pow(log10(energyBfCorrect), 2)
                                 + 0.4316E-02 * pow(log10(energyBfCorrect), 3);
      } else if (((*iShower).second.Theta() / PI * 180) < 180) {
        preliminaryCalibration = 0.9525 - 0.8955E-02 * pow(log10(energyBfCorrect), 1)
                                 - 0.1297E-01 * pow(log10(energyBfCorrect), 2)
                                 + 0.1134E-02 * pow(log10(energyBfCorrect), 3);
      } else {
        B2DEBUG(100, "ECLShower theta out of range " << ((*iShower).second.Theta() / PI * 180));

      }

      StoreArray<ECLShower> eclRecShowerArray;
      if (!eclRecShowerArray) eclRecShowerArray.create();
      m_hitNum = eclRecShowerArray->GetLast() + 1;
      new(eclRecShowerArray->AddrAt(m_hitNum)) ECLShower();
      eclRecShowerArray[m_hitNum]->setShowerId(nShower);
      eclRecShowerArray[m_hitNum]->setEnergy((float)(*iShower).second.Energy() / preliminaryCalibration);
      eclRecShowerArray[m_hitNum]->setTheta((float)(*iShower).second.Theta());
      eclRecShowerArray[m_hitNum]->setPhi((float)(*iShower).second.Phi());
      eclRecShowerArray[m_hitNum]->setR((float)(*iShower).second.Distance());
      eclRecShowerArray[m_hitNum]->setMass((float)(*iShower).second.Mass());
      eclRecShowerArray[m_hitNum]->setWidth((float)(*iShower).second.Width());
      eclRecShowerArray[m_hitNum]->setE9oE25((float)(*iShower).second.E9oE25());
      eclRecShowerArray[m_hitNum]->setE9oE25unf((float)(*iShower).second.E9oE25unf());
      eclRecShowerArray[m_hitNum]->setNHits((*iShower).second.NHits());
      eclRecShowerArray[m_hitNum]->setStatus((*iShower).second.Status());
      eclRecShowerArray[m_hitNum]->setGrade((*iShower).second.Grade());
      eclRecShowerArray[m_hitNum]->setUncEnergy((float)(*iShower).second.UncEnergy());
      double sEnergy = (*iShower).second.Energy();
      double sTheta = (*iShower).second.Theta();

      float ErrorMatrix[3] = {
        errorE(sEnergy),
        errorTheta(sEnergy, sTheta),
        errorPhi(sEnergy, sTheta)
      };
      eclRecShowerArray[m_hitNum]->setError(ErrorMatrix);

      nShower++;
//      if (((*iShower).second.Energy() > 0.05 && (*iShower).second.Energy() < 0.2)) {
//        checkflag = 1;
//      }



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
        ECLDigit* aECLDigi = eclDigiArray[ia];
        float FitEnergy    = (aECLDigi->getAmp()) / 20000;//ADC count to GeV
        int cId          =  aECLDigi->getCellId();
        ECLGeometryPar* eclp = ECLGeometryPar::Instance();
        TVector3 PosCell =  eclp->GetCrystalPos(cId);
        eclp->Mapping(cId);
        cout << "HitHA " << cId << " " << eclp->GetThetaID() << " " << eclp-> GetPhiID() << " " << FitEnergy << " "
             << PosCell.Theta() / PI * 180 << " " << PosCell.Phi() / PI * 180 << endl;
      }
    }

  */
  //cout<<"Event "<< m_nEvent<<" Total output number of Shower Array "<<++m_hitNum<<endl;
  m_nEvent++;
}

void ECLReconstructorModule::endRun()
{

  m_nRun++;
}

void ECLReconstructorModule::terminate()
{
  m_timeCPU = clock() * Unit::us - m_timeCPU;

}

float ECLReconstructorModule::errorE(double E)
{
  double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
//sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%
  return (float)sigmaE;

}
float ECLReconstructorModule::errorTheta(double Energy, double Theta)
{

  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

  if (Theta < theta_f) {
    return (float)(sigmaX * squ(cos(Theta)) / zForward);
  } else if (Theta > theta_b) {
    return (float)(sigmaX * squ(cos(Theta)) / (-1 * zBackward));
  } else {
    return (float)(sigmaX * sin(Theta) / Rbarrel)   ;
  }


}

float ECLReconstructorModule::errorPhi(double Energy, double Theta)
{

  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

  if (Theta < theta_f) {
    return (float)(sigmaX / (zForward * tan(Theta)))  ;
  } else if (Theta > theta_b) {
    return (float)(sigmaX / (zBackward * tan(Theta)))   ;
  } else {
    return (float)(sigmaX / Rbarrel);
  }



}




