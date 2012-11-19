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
#include "TFile.h"

#include <boost/lexical_cast.hpp>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


using namespace CLHEP;
using namespace std;


namespace Belle2 {




  EKLMFiberAndElectronics::EKLMFiberAndElectronics(pair < const G4VPhysicalVolume*,
                                                   vector<EKLMSimHit*> >
                                                   entry)
  {
    // get information from Gearbox
    GearDir Digitizer = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Digitizer");
    m_timeDigitizationStep = Digitizer.getInt("TimeDigitizationStep");
    m_nTimeDigitizationSteps = Digitizer.getInt("NTimeDigitizationSteps");
    m_nPEperMeV = Digitizer.getDouble("NPEperMeV");
    m_minCosTheta = cos(Digitizer.getAngle("MaxTheta"));
    m_mirrorReflectiveIndex = Digitizer.getDouble("MirrorReflectiveIndex");
    m_scintillatorDeExcitationTime = Digitizer.getDouble("ScintillatorDeExcitationTime");
    m_fiberDeExcitationTime = Digitizer.getDouble("FiberDeExcitationTime");
    m_outputFilename = Digitizer.getString("OutputFile");
    m_firstPhotonlightSpeed = Digitizer.getDouble("FirstPhotonSpeed");
    m_attenuationLength = Digitizer.getLength("AttenuationLength");
    m_expCoefficient = Digitizer.getDouble("SignalShapeExpCoefficient");
    m_meanSiPMNoise = Digitizer.getDouble("BackgroundPoissonMean");
    m_enableConstBkg = Digitizer.getDouble("EnableConstantBackgroundInTheFit");

    m_stripName = &entry.first->GetName();

    m_histRange = m_nTimeDigitizationSteps *   m_timeDigitizationStep;

    // create histos
    m_digitizedAmplitudeDirect = new TH1D("digitizedAmplitudeDirect", "",
                                          m_nTimeDigitizationSteps, 0,
                                          m_histRange);

    m_digitizedAmplitudeDirect->SetNameTitle("digitizedAmplitudeDirect",
                                             m_stripName->c_str());

    m_digitizedAmplitudeReflected = new TH1D("digitizedAmplitudeReflected", "",
                                             m_nTimeDigitizationSteps, 0,
                                             m_histRange);

    m_digitizedAmplitudeReflected->SetNameTitle("digitizedAmplitudeReflected",
                                                m_stripName->c_str());

    m_digitizedAmplitude = new TH1D("digitizedAmplitude", "",
                                    m_nTimeDigitizationSteps, 0,
                                    m_histRange);

    m_digitizedAmplitude->SetNameTitle("digitizedAmplitude", m_stripName->c_str());

    // define fit function
    m_fitFunction = new TF1("fitFunction", EKLMSignalShapeFitFunction, 0, m_histRange, 5);

    // define vector of hits
    m_vectorHits = entry.second;
  }


  EKLMFiberAndElectronics::~EKLMFiberAndElectronics()
  {
    delete     m_digitizedAmplitudeDirect;
    delete     m_digitizedAmplitudeReflected;
    delete     m_digitizedAmplitude;
    delete     m_fitFunction;
  }

  void EKLMFiberAndElectronics::processEntry()
  {

    for (vector<EKLMSimHit*> ::iterator iHit = m_vectorHits.begin();
         iHit != m_vectorHits.end(); iHit++) {

      // calculate distance
      lightPropagationDistance(*iHit);


      // Poisson mean for # of p.e.
      double nPEmean = (*iHit)->getEDep() * m_nPEperMeV;


      // fill histograms
      timesToShape(hitTimes(gRandom->Poisson(nPEmean), (*iHit)->getTime(),
                            false), m_digitizedAmplitudeDirect);
      if (m_mirrorReflectiveIndex > 0)
        timesToShape(hitTimes(gRandom->Poisson(nPEmean), (*iHit)->getTime(),
                              true), m_digitizedAmplitudeReflected);

    }

    // sum up histograms
    m_digitizedAmplitude->Add(m_digitizedAmplitudeDirect, 1);
    if (m_mirrorReflectiveIndex > 0)
      m_digitizedAmplitude->Add(m_digitizedAmplitudeReflected, 1);


    // add random SiPM noise to the histogram
    if (m_meanSiPMNoise > 0)
      addRandomSiPMNoise();

    // set up fit parameters
    m_fitFunction->SetParameters(10, 2., 0.04, 50, m_enableConstBkg);
    m_fitFunction->SetParLimits(0, 0, m_histRange);
    m_fitFunction->SetParLimits(1, 0, m_histRange);
    if (abs(m_enableConstBkg) < 0.0001)
      m_fitFunction->FixParameter(4, 0);

    // do fit
    m_fitResultsPtr = m_digitizedAmplitude->Fit(m_fitFunction, "LLSQB");


    // if save histograms if outputFilename is non-empty
    if (m_outputFilename.size() != 0) {
      const char* info = (string("Histograms will be saved with ") +
                          m_outputFilename +
                          string(" prefix. To switch it off change OutputFile parameter in EKLM.xml to void")).c_str();
      B2INFO(info);
      string filename = m_outputFilename + *m_stripName +
                        boost::lexical_cast<string>(gRandom->Integer(10000000)) + ".root";
      TFile* hfile = new TFile(filename.c_str(), "NEW");
      hfile->Append(m_digitizedAmplitudeDirect);
      hfile->Append(m_digitizedAmplitudeReflected);
      hfile->Append(m_digitizedAmplitude);
      hfile->Append(m_fitFunction);
      hfile->Write();
    } else {
      B2DEBUG(100, "OutputFile parameter in EKLM.xml is void. No histogram will be saved");
    }
  }


  //***********************************************************

  void EKLMFiberAndElectronics::lightPropagationDistance(EKLMSimHit* sh)
  {
    // actually, it is not too safe to use GetConstituentSolid(0).
    // here we rely on the fact that the first child is a real strip G4box
    // similar problem appears in EKLMRecon.cc
    G4Box* box = (G4Box*)(sh->getVolume()->GetLogicalVolume()->GetSolid()->GetConstituentSolid(0));
    // Native Geant4 units are mm, here we account for that
    double half_len = box->GetXHalfLength() * Unit::mm;
    double local_pos = sh->getLocalPosition()->x() ; // already in cm!!!
    m_hitDist = make_pair(half_len - local_pos, 3.0 * half_len + local_pos);
  }

  void EKLMFiberAndElectronics::addRandomSiPMNoise()
  {
    for (int iTimeStep = 0; iTimeStep < m_nTimeDigitizationSteps; iTimeStep++)
      m_digitizedAmplitude->AddBinContent(iTimeStep + 1,
                                          gRandom->Poisson(m_meanSiPMNoise));
  }

  double EKLMFiberAndElectronics::signalShape(double t)
  {
    if (t > 0)
      return exp(-m_expCoefficient * t);
    return 0;
  }

  double  EKLMFiberAndElectronics::distanceAttenuation(double dist)
  {
    return exp(-dist / m_attenuationLength);
  }

  vector<double> EKLMFiberAndElectronics::hitTimes(int nPE, double timeShift,
                                                   bool isReflected)
  {
    vector <double> hitTimesVector;
    // start selection procedure
    m_min_time = 100000000.;
    for (int i = 0; i < nPE; i++) {
      double cosTheta = gRandom->Uniform(m_minCosTheta, 1);
      double hitDist;
      if (!isReflected)
        hitDist = m_hitDist.first / cosTheta;
      else
        hitDist = m_hitDist.second / cosTheta;

      // drop lightflashes which was captured by fiber
      if (gRandom->Uniform() > distanceAttenuation(hitDist))
        continue;

      // account for mirror reflective index
      if (isReflected)
        if (gRandom->Uniform() > m_mirrorReflectiveIndex)
          continue;


      // Scintillator de-excitation time  && Fiber  de-excitation time
      double deExcitationTime = gRandom->Exp(m_scintillatorDeExcitationTime)
                                + gRandom->Exp(m_fiberDeExcitationTime);
      double hitTime = lightPropagationTime(hitDist) + deExcitationTime +
                       timeShift;
      // std::cout<<"TRMPORARY BUG IN EKLMFiberAndElectronics.cc"<<std::endl;
//       double hitTime = lightPropagationTime(hitDist) + timeShift;

      hitTimesVector.push_back(hitTime);
      if (hitTime < m_min_time)
        m_min_time = hitTime;
    }
    return hitTimesVector;
  }

  void EKLMFiberAndElectronics::timesToShape(const vector <double> & times,
                                             TH1D* shape)
  {
    //    for (unsigned  i = 0; i < times.size(); i++)
    for (std::vector<double>::const_iterator i = times.begin();
         i != times.end(); i++)
      for (int iTimeStep = 0; iTimeStep < m_nTimeDigitizationSteps; iTimeStep++)
        shape->AddBinContent(iTimeStep + 1,
                             signalShape(iTimeStep * m_timeDigitizationStep -
                                         //                                         times[i]));
                                         *i));
  }

  double EKLMFiberAndElectronics::lightPropagationTime(double L)
  {
    return L / m_firstPhotonlightSpeed;
  }


  TFitResultPtr EKLMFiberAndElectronics::getFitResultsPtr() const
  {
    return m_fitResultsPtr;
  }

  double EKLMFiberAndElectronics::getFitResults(int i) const
  {
    if (i == -1)
      return m_min_time;
    return m_fitResultsPtr->Value(i);
  }

  int EKLMFiberAndElectronics::getFitStatus() const
  {
    return (int)m_fitResultsPtr;
  }



  //----------------------------------------------------------



  double EKLMSignalShapeFitFunction(double* _x, double* par)
  {
    double x = *_x;
    double u = 0;

    if (x > par[0] && x < par[0] + par[1])
      u = par[3] / par[1] * (x - par[0]);
    else if (x > par[0] + par[1])
      u = par[3] * exp(-par[2] * (x - par[0] - par[1]));

    return u + par[4];
  }


}

