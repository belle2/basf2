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

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

int Event = 0;
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

  //Default values of these exist in HE3TUBE.xml. If the user prefers, they can try different drift data files or impuse responses.
  addParam("driftDataFile", m_driftDataFile, "File containing Garfield generated drift data. Default taken from HE3TUBE.xml", string(""));
  addParam("impulseSigma", m_impulseSigma, "Sigma of the gaussian used as the impulse response of the detector (ns). Default taken from HE3TUBE.xml", -1.);


}

He3DigitizerModule::~He3DigitizerModule()
{
}

void He3DigitizerModule::initialize()
{
  B2INFO("Initializing he3digitizer");
  StoreArray<He3tubeHit>::registerPersistent();

  //get the garfield drift data and create impulse response function
  getXMLData();
  getDriftData();
  impulseResponse();

}

void He3DigitizerModule::beginRun()
{
}

void He3DigitizerModule::event()
{

  //there are five arrays used here that are declared in the header file:
  // waveform[8][10000]                 - The raw waveform containing pulseheights
  // convolvedWaveform[8][10000]        - Waveform convolved with impulse response
  // peak[8]                            - peak of the convolved waveform
  // TubeCenterX[8] & TubeCenterY[8]    - x,y coordinate of tube centers

  StoreArray<MCParticle> particles;
  StoreArray<He3tubeSimHit> He3SimHits;
  StoreArray<He3tubeHit> He3Hits;

  //array size must be big for all the events in a tube
  double edepArray[8][50000] = {{0}};
  double timeArray[8][50000] = {{0}};

  //skip events with no He3SimHits, but continue the event counter
  if (He3SimHits.getEntries() == 0) {
    Event++;
    return;
  }

  double lowTime[8];
  std::fill_n(lowTime, 8, 9999999);

  int numInDet[8] = {0};


  double edepDet[8] = {0};
  for (int i = 0; i < He3SimHits.getEntries(); i++) {
    He3tubeSimHit* aHit = He3SimHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    TVector3 posn = aHit->gettkPos();
    double x = posn.X() / 100.;
    double y = posn.Y() / 100.;

    double r = sqrt((x - TubeCenterX[detNB]) * (x - TubeCenterX[detNB]) + (y - TubeCenterY[detNB]) * (y - TubeCenterY[detNB])); //find radius of the energy deposit
    double driftTime = getDriftTime(r);                                                                              //get drift time
    double time = (aHit->getGlTime() + driftTime * 1000.);                                                            //time when energy drifts to the wire

    //save energy deposited and time to an array, to create waveforms
    edepArray[detNB][numInDet[detNB]] = edep;
    timeArray[detNB][numInDet[detNB]] = time;

    //number of events in each tube
    numInDet[detNB]++;

    //find earliest time in each tube
    if (time < lowTime[detNB]) lowTime[detNB] = time;

    edepDet[detNB] = edepDet[detNB] + edep;
  }


  //create waveforms
  for (int detNB = 0; detNB < 8; detNB++) {
    for (int j = 0; j < numInDet[detNB]; j++) {
      int t = int(timeArray[detNB][j] - lowTime[detNB]); //subtract earliest time to get a waveform starting at t=0
      waveform[detNB][t + 3000] += edepArray[detNB][j];
    }

    //convolve waveform, find peak
    convolveWaveform(detNB);
    findPeak(detNB);

  }


  for (int i = 0; i < 8; i++) {
    if (edepDet[i] != 0) {
      He3tubeHit* a = He3Hits.appendNew(He3tubeHit(edepDet[i], i, peak[i], lowTime[i]));                         //create He3tubeHit
      if (peak[i] > 0.09) B2INFO(edepDet[i] << " deposited in tube #" << i << " with waveform peak of " << peak[i]); //if peak is likely from a neutron hit, print to console
    }
  }


  //reset arrays
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < waveformSize; i++) {
      convolvedWaveform[j][i] = 0;
      waveform[j][i] = 0;
    }
    peak[j] = 0;
  }


  Event++;
}

//read Garfield generated file containing drift time vs radius
void He3DigitizerModule::getDriftData()
{

  B2INFO("Reading drift data from: " << m_driftDataFile);

  ifstream InFile;
  InFile.open(m_driftDataFile);
  if (!InFile) {
    B2FATAL("Drift data file " << m_driftDataFile << " not found!")
    return;
  } else {
    for (int i = 0; i < 248; i++) {
      InFile >> radius_drift[i];
      InFile >> time_drift[i];
    }
  }
  InFile.close();
}

//returns drift time for a given radius from the wire
double He3DigitizerModule::getDriftTime(double R)
{

  int lowIndex = 0;
  for (int i = 0; i < 248; i++) {
    if (radius_drift[i] < R) lowIndex = i;
  }

  if (lowIndex == 247) return time_drift[247];

  //interpolate the drift time from the nearest entries in time_drift[]
  return time_drift[lowIndex] + (time_drift[lowIndex + 1] - time_drift[lowIndex]) * (R - radius_drift[lowIndex]) / (radius_drift[lowIndex + 1] - radius_drift[lowIndex]);


}

//create a gaussian impulse response function
void He3DigitizerModule::impulseResponse()
{
  for (int i = 0; i < 2400; i++) {
    double t = i - 1200;
    iResponse[i] = exp(-t * t / (2.*m_impulseSigma * m_impulseSigma));
  }

}

//convolve waveform with impulse response
void He3DigitizerModule::convolveWaveform(int detNB)
{
  for (int n = 2400; n < (waveformSize); n++) {
    for (int m = 0; m < 2400; m++) {
      convolvedWaveform[detNB][n - 2000] += waveform[detNB][n - m] * iResponse[m];
    }
  }
}

//would like to implement caen digitizer method of peak finding eventually
void He3DigitizerModule::findPeak(int detNB)
{
  for (int i = 0; i < waveformSize; i++) {
    if (convolvedWaveform[detNB][i] > peak[detNB]) peak[detNB] = convolvedWaveform[detNB][i];
  }
}

//prints the convolved waveforms to a text file. Useful for debugging
void He3DigitizerModule::printConvWaveform(string fileName)
{
  B2INFO("Creating waveform file: " << fileName);
  ofstream outputFile;
  outputFile.open(fileName);
  for (int detNB = 0; detNB < 8; detNB++) {
    for (int i = 0; i < waveformSize; i++) {
      outputFile << detNB << "\t" << i << "\t" << convolvedWaveform[detNB][i] << endl;
    }
  }
  outputFile.close();
}

//read tube centers, impulse response, and garfield drift data filename from HE3TUBE.xml
void He3DigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"HE3TUBE\"]/Content/");

  int i = 0;
  //get the location of the 8 tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
    TubeCenterX[i] = activeParams.getLength("x_he3tube") ;
    TubeCenterY[i] = activeParams.getLength("y_he3tube") ;
    i++;
  }

  if (m_driftDataFile == "") m_driftDataFile = content.getString("GarfieldFile");
  if (m_impulseSigma == -1.) m_impulseSigma = content.getDouble("impulseSigma");

  B2INFO("Aquired tube locations, garfield drift data filename (" << m_driftDataFile << "), and impulse sigma (" << m_impulseSigma << ") from HE3TUBE.xml");

}


void He3DigitizerModule::endRun()
{
}

void He3DigitizerModule::terminate()
{
}


