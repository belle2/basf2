#include <topcaf/modules/TimeBasedCorrectionModule/TimeBasedCorrectionModule.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <fstream>

using namespace Belle2;
using namespace std;

REG_MODULE(TimeBasedCorrection);

TimeBasedCorrectionModule::TimeBasedCorrectionModule() : Module()
{
  setDescription("This module applies the time-based correction for modules 3 and 4");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 47.163878);
  addParam("correctionFileName", m_correctionFilename, "Name of the file containing the time-based correction factors",
           string("/group/belle2/testbeam/TOP/M03_M04_dT.dat"));
  addParam("moduleNumber", m_moduleNumber, "Number of the module for the correction (3 or 4)", 3);
  m_topConfig = StoreObjPtr<TopConfigurations>("", DataStore::c_Persistent);
}


TimeBasedCorrectionModule::~TimeBasedCorrectionModule() {}


void TimeBasedCorrectionModule::initialize()
{
  if (m_moduleNumber != 3 && m_moduleNumber != 4) {
    B2FATAL("Can only correct modules 3 and 4");
  }
}

const int NTOPMOD = 16;
const int NBS = 4;
const int NCAR = 4;
const int NASIC = 4;
const int NCHAN = 8;
const int NDT = 256;

void TimeBasedCorrectionModule::beginRun()
{
  // SSTin determined timing parameters
  // const float winDt = 47.163878; // ns per window  (21.2MHz)  6000/127.216 MHz
  // const float winDt2 = 2.0*winDt; // ns per window
  // const float nomDt = winDt2/255.0; // ns @ 2.8 GSPS

  // Master dT array
  // the array is too big for the stack (Belle2 stack warning). So we create on the heap.
  m_correctionArray = new float[NTOPMOD * NBS * NCAR * NASIC * NCHAN * NDT];

  ifstream in;
  in.open(m_correctionFilename.c_str());
  if (not in) {
    B2FATAL("Could not find file for time correction");
  }
  // Input dT File -- total TOP subsystem
  // note:  offset of 1 for Module enumeration
  for (int iNTM = 3; iNTM < 5; iNTM++) {
    for (int iBS = 0; iBS < NBS; iBS++) {
      for (int iCar = 0; iCar < NCAR; iCar++) {
        for (int iASIC = 0; iASIC < NASIC; iASIC++) {
          for (int iCH = 0; iCH < NCHAN; iCH++) {
            for (int iDT = 0; iDT < NDT; iDT++) {
              if (in.eof()) {
                B2FATAL("Correction File ended prematurely");
              }
              in >> m_correctionArray[iNTM * NTOPMOD + iBS * NBS + iCar * NCAR + iASIC * NASIC + iCH * NCHAN + iDT * NDT];
            }
          }
        }
      }
    }
  }
  in.close();
}


void TimeBasedCorrectionModule::event()
{
  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves;
  evtwaves.isRequired();
  m_topConfig.isRequired();

  for (int w = 0; w < evtwaves.getEntries(); w++) {
    EventWaveformPacket* wp = evtwaves[w];
    topcaf_channel_id_t channel_id = wp->GetChannelID();
    int column = wp->GetASICColumn();
    int row = wp->GetASICRow(); // same as carrier
    int window = wp->GetASICWindow();
    int channel = wp->GetASICChannel();
    unsigned int scrodID = wp->GetScrodID();
    int boardstack = m_topConfig->scrod_to_electronicsModuleNumber(scrodID);
    vector<double> samples = wp->GetSamples();
    for (size_t si = 0; si < samples.size(); ++si) {
      samples[si] += m_correctionArray[m_moduleNumber * NTOPMOD + boardstack * NBS + row * NCAR + column * NASIC + channel * NCHAN + si *
                                       NDT];
    }
    wp->SetSamples(samples);
  }
}

void TimeBasedCorrectionModule::terminate()
{

}
