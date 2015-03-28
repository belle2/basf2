/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/he3tube/modules/He3DigitizerModule.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>
#include <beast/he3tube/dataobjects/He3tubeHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace std;
using namespace Belle2;
using namespace he3tube;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(He3Digitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

He3DigitizerModule::He3DigitizerModule() : Module()
{
  // Set module properties
  setDescription("He3tube digitizer module");

  //Default values of these exist in HE3TUBE.xml. If the user prefers, they can try different drift data files or impulse responses.
  addParam("driftDataFile", m_driftDataFile,
           "File containing Garfield generated drift data. If no file is specified, data from HE3TUBE.xml is used.", string(""));
  addParam("impulseSigma", m_impulseSigma,
           "Sigma of the gaussian used as the impulse response of the detector (ns). Default taken from HE3TUBE.xml", -1.);


}

He3DigitizerModule::~He3DigitizerModule()
{
}

void He3DigitizerModule::initialize()
{
  B2INFO("He3Digitizer: Initializing");
  StoreArray<He3tubeHit>::registerPersistent();

  //get the garfield drift data and create impulse response function
  getXMLData();
  if (m_driftDataFile != "") getDriftData();
  impulseResponse();

}

void He3DigitizerModule::beginRun()
{
}

void He3DigitizerModule::event()
{

  //there are two vectors used here that are declared in the header file:
  // TubeCenterX[] & TubeCenterY[]     - x,y coordinate of tube centers, taken from HE3TUBE.xml

  StoreArray<MCParticle> particles;
  StoreArray<He3tubeSimHit> He3SimHits;
  StoreArray<He3tubeHit> He3Hits;

  //skip events with no He3SimHits, but continue the event counter
  if (He3SimHits.getEntries() == 0) {
    Event++;
    return;
  }

  auto edepArray = new vector<double>[numOfTubes](); //all the energy deposits
  auto timeArray = new vector<double>[numOfTubes](); //time of energy deposits


  auto lowTime = new double[numOfTubes]();  //earliest deposit in each tube
  std::fill_n(lowTime, numOfTubes, 9999999);
  auto numInDet = new int[numOfTubes]();    //number of hits in each tube
  auto edepDet = new double[numOfTubes]();  //total energy deposited per tube

  int nentries = He3SimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    He3tubeSimHit* aHit = He3SimHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    if (detNB > numOfTubes) {
      B2WARNING("He3Digitizer: Detector number of He3tubeSimHit is greater than number implemented! Ignoring He3tubeSimHit.");
      break;
    }
    TVector3 posn = aHit->gettkPos();
    double x = posn.X() / 100.;
    double y = posn.Y() / 100.;

    double r = sqrt((x - TubeCenterX[detNB]) * (x - TubeCenterX[detNB]) + (y - TubeCenterY[detNB]) *
                    (y - TubeCenterY[detNB])); //find radius of the energy deposit
    double driftTime = getDriftTime(
                         r);                                                                                         //get drift time
    double time = (aHit->getGlTime() +
                   driftTime);                                                                      //time when energy drifts to the wire

    //save energy and time for waveform creation
    edepArray[detNB].push_back(edep);
    timeArray[detNB].push_back(time);

    //number of events in each tube
    numInDet[detNB]++;

    //find earliest time in each tube
    if (time < lowTime[detNB]) lowTime[detNB] = time;

    edepDet[detNB] = edepDet[detNB] + edep;
  }


  auto waveform = new double[numOfTubes][waveformSize]();               //raw waveform
  auto peak = new double[numOfTubes]();                                 //peak of waveform

  //create waveforms
  for (int detNB = 0; detNB < numOfTubes; detNB++) {
    if (edepDet[detNB] == 0) { //don't bother creating a waveform when there's no energy deposits in the tube
      continue;
    }
    for (int j = 0; j < numInDet[detNB]; j++) {
      int t = int(timeArray[detNB][j] - lowTime[detNB]); //subtract earliest time to get a waveform starting at t=0
      if ((t + 3000) > waveformSize) {
        B2WARNING("He3Digitizer: Energy deposit occuring after waveform time window. Time = " << (t + 3000) << ", Event # " << Event);
        break;
      } else {
        waveform[detNB][t + 3000] += edepArray[detNB][j];
      }
    }

    //convolve waveform, find peak
    peak[detNB] = convolveWaveform(waveform[detNB]);

  }



  for (int i = 0; i < numOfTubes; i++) {
    if (edepDet[i] != 0) {
      //create He3tubeHit
      He3Hits.appendNew(He3tubeHit(edepDet[i], i, peak[i], lowTime[i]));

      //if peak is likely from a neutron hit, print to console (only in debug mode)
      if (peak[i] > 0.09) B2DEBUG(80, "He3Digitizer: " << edepDet[i] << "MeV deposited in tube #" << i << " with waveform peak of " <<
                                    peak[i]);
    }
  }

  Event++;

  //delete arrays
  delete[] peak;
  delete[] edepArray;
  delete[] timeArray;
  delete[] lowTime;
  delete[] numInDet;
  delete[] edepDet;
  delete[] waveform;

}

//read Garfield generated file containing drift time vs radius
void He3DigitizerModule::getDriftData()
{

  B2INFO("He3Digitizer: Reading drift data from: " << m_driftDataFile);

  ifstream InFile;
  InFile.open(m_driftDataFile);
  if (!InFile) {
    B2WARNING("He3Digitizer: Drift data file " << m_driftDataFile << " not found, using default in HE3TUBE.xml");
    return;
  } else {
    radius_drift.clear();
    time_drift.clear();
    for (int i = 0; i < 248; i++) {
      double r;
      double t;
      InFile >> r;
      InFile >> t;
      radius_drift.push_back(r);
      time_drift.push_back(t);
    }
  }
  InFile.close();
}

//returns drift time for a given radius from the wire
double He3DigitizerModule::getDriftTime(double R)
{

  int lowIndex = -1;
  std::vector<double>::iterator it;
  it = lower_bound(radius_drift.begin(), radius_drift.end(), R);
  lowIndex = (it - 1) - radius_drift.begin();

  if (lowIndex == 247) return time_drift[247];

  //interpolate the drift time from the nearest entries in time_drift[]
  double drift = time_drift[lowIndex] + (time_drift[lowIndex + 1] - time_drift[lowIndex]) * (R - radius_drift[lowIndex]) /
                 (radius_drift[lowIndex + 1] - radius_drift[lowIndex]);
  return drift;


}

//create a gaussian impulse response function
void He3DigitizerModule::impulseResponse()
{
  for (int i = 0; i < 2400; i++) {
    double t = i - 1200;
    iResponse[i] = exp(-t * t / (2.*m_impulseSigma * m_impulseSigma));
  }

}

//convolve waveform with impulse response, return peak
double He3DigitizerModule::convolveWaveform(double* waveform)
{
  double peak = 0;
  double convolve = 0;
  for (int n = 2400; n < (waveformSize); n++) {
    for (int m = 0; m < 2400; m++) {
      convolve += waveform[n - m] * iResponse[m];
    }
    if (convolve > peak) peak = convolve;
    convolve = 0;

  }
  return peak;
}

//prints the convolved waveforms to a text file. Useful for debugging
void He3DigitizerModule::printConvWaveform(int eventNum, int detNB, double* convolvedWaveform)
{
  std::ostringstream s;
  s << eventNum;
  string fileName = "Trace_Event_" + s.str() + ".txt";

  B2INFO("He3Digitizer: Creating waveform file: " << fileName);
  ofstream outputFile;
  outputFile.open(fileName);
  for (int i = 0; i < waveformSize; i++) {
    outputFile << detNB << "\t" << i << "\t" << convolvedWaveform[i] << endl;
  }
  outputFile.close();
}


//read tube centers, impulse response, and garfield drift data filename from HE3TUBE.xml
void He3DigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"HE3TUBE\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    TubeCenterX.push_back(activeParams.getLength("x_he3tube"));
    TubeCenterY.push_back(activeParams.getLength("y_he3tube"));

    numOfTubes++;
  }

  //drift data
  radius_drift = content.getArray("DriftRadius");
  time_drift = content.getArray("DriftTime");

  if (m_impulseSigma == -1.) m_impulseSigma = content.getDouble("impulseSigma");

  B2INFO("He3Digitizer: Aquired tube locations, drift data, and impulse sigma (" << m_impulseSigma << ")");
  B2INFO("              from HE3TUBE.xml. There are " << numOfTubes << " tubes implemented");

}

void He3DigitizerModule::endRun()
{
}

void He3DigitizerModule::terminate()
{
}


