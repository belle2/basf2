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
  int hitNum = eclDigiArray.getEntries();

  TEclEnergyHit ss;
  for (int ii = 0; ii < hitNum; ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    //float FitTime      =  (1520 - aECLDigi->getTimeFit())*24.*12/508/(3072/2) ;//in us

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
        //m_HANum = eclHaArray->GetLast() + 1;
        //new(eclHaArray->AddrAt(m_HANum)) ECLHitAssignment();
        new(eclHaArray.nextFreeAddress()) ECLHitAssignment();
        m_HANum = eclHaArray.getEntries() - 1;

        eclHaArray[m_HANum]->setShowerId(nShower);
        eclHaArray[m_HANum]->setCellId(iHA->Id());

//        cout<<iHA->Id()<<" ";
      }
//        cout<<endl;

      double energyBfCorrect = (*iShower).second.Energy();
      double preliminaryCalibration = correctionFactor(energyBfCorrect, (*iShower).second.Theta()) ;
      double sEnergy = (*iShower).second.Energy() / preliminaryCalibration;


      StoreArray<ECLShower> eclRecShowerArray;
      if (!eclRecShowerArray) eclRecShowerArray.create();
      //m_hitNum = eclRecShowerArray->GetLast() + 1;
      //new(eclRecShowerArray->AddrAt(m_hitNum)) ECLShower();
      new(eclRecShowerArray.nextFreeAddress()) ECLShower();
      m_hitNum = eclRecShowerArray.getEntries() - 1;
      eclRecShowerArray[m_hitNum]->setShowerId(nShower);
      eclRecShowerArray[m_hitNum]->setEnergy((float) sEnergy);
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

  double sigmaE = 0.01 * (0.047 / E + 1.105 / sqrt(sqrt(E)) + 0.8563) * E;
//double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
//sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%//NIM A441, 401(2000)
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



double ECLReconstructorModule::correctionFactor(double Energy, double Theta)
{

  int  eCorrId = -1;
  if (Theta > 0 && (Theta / PI * 180) < 32.2) {
    eCorrId = 0;
  } else if ((Theta / PI * 180) >= 32.2 && (Theta / PI * 180) < 35.0) {
    eCorrId = 1;
  } else if ((Theta / PI * 180) >= 35.0 && (Theta / PI * 180) < 37.5) {
    eCorrId = 2;
  } else if ((Theta / PI * 180) >= 37.5 && (Theta / PI * 180) < 40.0) {
    eCorrId = 3;
  } else if ((Theta / PI * 180) >= 40.0 && (Theta / PI * 180) < 42.5) {
    eCorrId = 4;
  } else if ((Theta / PI * 180) >= 42.5 && (Theta / PI * 180) < 45.0) {
    eCorrId = 5;
  } else if ((Theta / PI * 180) >= 45.0 && (Theta / PI * 180) < 55.0) {
    eCorrId = 6;
  } else if ((Theta / PI * 180) >= 55.0 && (Theta / PI * 180) < 65.0) {
    eCorrId = 7;
  } else if ((Theta / PI * 180) >= 65.0 && (Theta / PI * 180) < 75.0) {
    eCorrId = 8;
  } else if ((Theta / PI * 180) >= 75.0 && (Theta / PI * 180) < 85.0) {
    eCorrId = 9;
  } else if ((Theta / PI * 180) >= 85.0 && (Theta / PI * 180) < 95.0) {
    eCorrId = 10;
  } else if ((Theta / PI * 180) >= 95.0 && (Theta / PI * 180) < 105.0) {
    eCorrId = 11;
  } else if ((Theta / PI * 180) >= 105.0 && (Theta / PI * 180) < 115.0) {
    eCorrId = 12;
  } else if ((Theta / PI * 180) >= 115.0 && (Theta / PI * 180) < 128.7) {
    eCorrId = 13;
  } else if ((Theta / PI * 180) >= 128.7 && (Theta / PI * 180) <= 180) {
    eCorrId = 14;

  } else {
    B2ERROR("ECLShower theta out of range " << (Theta / PI * 180));

  }




  double energyCorrectPolynomial[15][4] = {
    {9.56670E-01, -2.28268E-03, -2.23364E-03, 2.41601E-04},
    {9.39221E-01, -6.01211E-03, -1.01388E-03, 2.21742E-04},
    {9.47695E-01, -4.00680E-03, -6.62361E-04, 3.51879E-04},
    {9.46688E-01, -2.83932E-03, -6.50563E-04, 2.73304E-04},
    {9.51124E-01, -3.64272E-03, -1.03425E-03, 2.14907E-04},
    {9.53309E-01, -2.62172E-03, -8.97187E-04, 3.51823E-04},
    {9.53309E-01, -2.62172E-03, -8.97187E-04, 3.51823E-04},
    {9.58557E-01, -2.44004E-03, -1.49555E-03, 1.41088E-04},
    {9.60535E-01, -2.77543E-03, -1.44099E-03, 2.44692E-04},
    {9.61075E-01, -3.09389E-03, -1.45289E-03, 2.16016E-04},
    {9.60538E-01, -3.35572E-03, -1.36629E-03, 2.30296E-04},
    {9.61748E-01, -4.30415E-03, -1.05225E-03, 3.59288E-04},
    {9.61925E-01, -4.44564E-03, -1.13480E-03, 2.55418E-04},
    {9.58142E-01, -4.75945E-03, -6.66410E-04, 4.77996E-04},
    {9.49364E-01, -3.31338E-03, -2.25274E-03, 1.43136E-04}
  };




  double  preliminaryCalibration = energyCorrectPolynomial[eCorrId][0]
                                   + energyCorrectPolynomial[eCorrId][1] * pow(log(Energy), 1)
                                   + energyCorrectPolynomial[eCorrId][2] * pow(log(Energy), 2)
                                   + energyCorrectPolynomial[eCorrId][3] * pow(log(Energy), 3);


  return preliminaryCalibration;

}

