/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/simeklm/EKLMFiberAndElectronics.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>

#include "G4Box.hh"
#include "TRandom.h"
#include "TH1D.h"

using namespace CLHEP;


namespace Belle2 {

  double EKLMFiberAndElectronics::getFitResults(int i)
  {
    return fitResultsPtr->Value(i);
  }

  int EKLMFiberAndElectronics::getFitStatus()
  {
    return (int)fitResultsPtr;
  }

  EKLMFiberAndElectronics::EKLMFiberAndElectronics(std::pair < std::string,
                                                   std::vector<EKLMSimHit*> >
                                                   entry)
  {
    timeDigitizationStep = 1; // 1 ns
    nTimeDigitizationSteps = 200; // 1000 steps
    nPEperMeV = 20;
    // alpha=arccos(1-Sangle/2/pi)
    // Sangle=5.5%*2pi
    minCosTheta = 0.9912; // Suspicious! to be checked!!!
    mirrorReflectiveIndex = 0.95;  // to be refined
    scintillatorDeExcitationTime = 3; //ns
    fiberDeExcitationTime = 10.; //ns

    const char * stripName = entry.first.c_str();
    digitizedAmplitudeDirect = new TH1D("digitizedAmplitudeDirect", "",
                                        nTimeDigitizationSteps, 0,
                                        nTimeDigitizationSteps *
                                        timeDigitizationStep);
    digitizedAmplitudeDirect->SetNameTitle("digitizedAmplitudeDirect",
                                           stripName);
    digitizedAmplitudeReflected = new TH1D("digitizedAmplitudeReflected", "",
                                           nTimeDigitizationSteps, 0,
                                           nTimeDigitizationSteps *
                                           timeDigitizationStep);
    digitizedAmplitudeReflected->SetNameTitle("digitizedAmplitudeReflected",
                                              stripName);
    digitizedAmplitude = new TH1D("digitizedAmplitude", "",
                                  nTimeDigitizationSteps, 0,
                                  nTimeDigitizationSteps *
                                  timeDigitizationStep);
    digitizedAmplitude->SetNameTitle("digitizedAmplitude", stripName);


    fitFunction = new TF1("fitFunction", EKLMSignalShapeFitFunction, 0, 300, 4);
    vectorHits = entry.second;
  }


  void EKLMFiberAndElectronics::processEntry()
  {

    for (std::vector<EKLMSimHit*> ::iterator iHit = vectorHits.begin();
         iHit != vectorHits.end(); iHit++) {

      // calculate distance
      lightPropagationDistance(forwardHitDist, backwardHitDist, *iHit);

      // calculate # of p.e.
      int nForwardPE = gRandom->Poisson((*iHit)->getEDep() * nPEperMeV);
      int nBackwardPE = gRandom->Poisson((*iHit)->getEDep() * nPEperMeV);

      hitTimes(nForwardPE, false);
      hitTimes(nBackwardPE, true);

      timesToShape(&hitTimesVectorBackward, digitizedAmplitudeReflected);
      timesToShape(&hitTimesVectorForward, digitizedAmplitudeDirect);

      digitizedAmplitude->Add(digitizedAmplitudeReflected, 1);
      digitizedAmplitude->Add(digitizedAmplitudeDirect, 1);


      fitFunction->SetParameters(10, 2., 0.04, 30);
      fitResultsPtr = digitizedAmplitude->Fit(fitFunction, "LLSQ");

    }

  }

  EKLMFiberAndElectronics::~EKLMFiberAndElectronics()
  {
    delete     digitizedAmplitudeDirect;
    delete     digitizedAmplitudeReflected;
    delete     digitizedAmplitude;
    delete     fitFunction;
  }

  //***********************************************************

  void EKLMFiberAndElectronics::lightPropagationDistance(double &firstHitDist,
                                                         double &secondHitDist,
                                                         EKLMSimHit *sh)
  {
    G4Box *box = (G4Box*)(sh->getPV()->GetLogicalVolume()->GetSolid());
    double half_len = box->GetXHalfLength();
    firstHitDist = half_len - (sh->getLocalPos()).x();   //  direct light hit
    secondHitDist = 4.0 * half_len - firstHitDist;     //  reflected light hit
  }

  double EKLMFiberAndElectronics::addRandomNoise(double ampl)
  {
    // Gauss for now
    return gRandom->Gaus(ampl, sqrt(ampl));
  }

  double EKLMFiberAndElectronics::signalShape(double t)
  {
    double expCoefficient = 0.04;
    if (t > 0)
      return exp(-expCoefficient*t);
    return 0;
  }

  double  EKLMFiberAndElectronics::distanceAttenuation(double dist)
  {
    //temporary  : att.Length=3 m
    return exp(-dist / 300);
  }

  void EKLMFiberAndElectronics::hitTimes(int nPE, bool isReflected)
  {
    // start selection procedure
    for (int i = 0; i < nPE; i++) {
      double cosTheta = gRandom->Uniform(minCosTheta, 1);
      double hitDist;
      if (isReflected)
        hitDist = backwardHitDist / cosTheta;
      else
        hitDist = forwardHitDist / cosTheta;
      // drop lightflashes which was captured by fiber
      if (gRandom->Uniform() > distanceAttenuation(hitDist))
        continue;

      // account for mirror reflective index
      if (isReflected)
        if (gRandom->Uniform() > mirrorReflectiveIndex)
          continue;


      // Scintillator de-excitation time  && Fiber  de-excitation time
      double deExcitationTime = gRandom->Exp(scintillatorDeExcitationTime)
                                + gRandom->Exp(fiberDeExcitationTime);
      double hitTime = lightPropagationTime(hitDist) + deExcitationTime;
      if (isReflected)
        hitTimesVectorBackward.push_back(hitTime);
      else
        hitTimesVectorForward.push_back(hitTime);
    }
  }

  void EKLMFiberAndElectronics::timesToShape(std::vector <double> * times,
                                             TH1D * shape)
  {
    for (unsigned  i = 0; i < times->size(); i++)
      for (int iTimeStep = 0; iTimeStep < nTimeDigitizationSteps; iTimeStep++)
        shape->AddBinContent(iTimeStep + 1,
                             signalShape(iTimeStep*timeDigitizationStep -
                                         (*times)[i]));
  }

  double EKLMFiberAndElectronics::lightPropagationTime(double L)
  {
    // nearly arbitrary function. to be updated
    double speed = 17;// (cm/ns)   // should be accessible via xml!
    return L / speed;
  }
  //----------------------------------------------------------



  double EKLMSignalShapeFitFunction(double *_x, double * par)
  {
    double x = *_x;
    double u = 0;

    if (x > par[0] && x < par[0] + par[1])
      u = par[3] / par[1] * (x - par[0]);
    else if (x > par[0] + par[1])
      u = par[3] * exp(-par[2] * (x - par[0] - par[1]));

    return u;
  }


}

