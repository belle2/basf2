//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DConverterModule.cc
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCT3DConverter Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------


#include <trg/cdc/modules/trgcdct3dConverter/TRGCDCT3DConverterModule.h>
#include <bitset>
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JSignalData.h"
#include "boost/multi_array.hpp"

using namespace boost;
using namespace std;
using namespace Belle2;
using namespace TRGCDCT3DCONVERTERSPACE;
//! Register Module
REG_MODULE(TRGCDCT3DConverter);

string TRGCDCT3DConverterModule::version() const
{
  return string("1.00");
}

TRGCDCT3DConverterModule::TRGCDCT3DConverterModule()
  : Module::Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  string desc = "TRGCDCT3DConverterModule(" + version() + ")";
  setDescription(desc);
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string("CDCTriggerSegmentHits0"));
  addParam("addTSToDatastore", m_addTSToDatastore,
           "If true, adds TS to datastore",
           true);
  addParam("EventTimeName", m_EventTimeName,
           "Name of the event time object.",
           string("BinnedEventT00"));
  addParam("addEventTimeToDatastore", m_addEventTimeToDatastore,
           "If true, adds event time to datastore",
           true);
  addParam("inputCollectionName", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D finder.",
           string("TRGCDC2DFinderTracks0"));
  addParam("add2DFinderToDatastore", m_add2DFinderToDatastore,
           "If true, adds 2D Finder results to datastore",
           true);
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("TRGCDC3DFitterTracks0"));
  addParam("add3DToDatastore", m_add3DToDatastore,
           "If true, adds 3D results to datastore",
           true);
  addParam("fit3DWithTSIM", m_fit3DWithTSIM,
           "0: use firmware results. 1: Fits with fast TSIM. 2: Fits with firm TSIM",
           unsigned(0));
  addParam("firmwareResultCollectionName", m_firmwareResultCollectionName,
           "Name of the StoreArray holding the firmware results.",
           string("TRGCDCT3DUnpackerStores0"));
  addParam("isVerbose", m_isVerbose,
           "If not zero, prints detail information.",
           unsigned(0));
  B2INFO("TRGCDCT3DConverter: Constructor done.");
}

TRGCDCT3DConverterModule::~TRGCDCT3DConverterModule()
{
}

void TRGCDCT3DConverterModule::terminate()
{
}

void TRGCDCT3DConverterModule::initialize()
{
  m_firmwareResults.isRequired(m_firmwareResultCollectionName);
  if (m_add3DToDatastore) m_tracks3D.registerInDataStore(m_outputCollectionName);
  if (m_add2DFinderToDatastore) m_tracks2D.registerInDataStore(m_inputCollectionName);
  if (m_addTSToDatastore) m_hits.registerInDataStore(m_hitCollectionName);
  if (m_addEventTimeToDatastore) m_eventTime.registerInDataStore(m_EventTimeName);

  if (m_add2DFinderToDatastore && m_addTSToDatastore) m_tracks2D.registerRelationTo(m_hits);
  if (m_add2DFinderToDatastore && m_add3DToDatastore) m_tracks3D.registerRelationTo(m_tracks2D);

  m_commonData = new Belle2::TRGCDCJSignalData();
}

void TRGCDCT3DConverterModule::beginRun()
{
}

void TRGCDCT3DConverterModule::endRun()
{
}

void TRGCDCT3DConverterModule::event()
{
  //cout<<"Event"<<endl;

  if (m_addTSToDatastore) {
    // Process firmware stereo TS
    // stTsfFirmwareInfo[stSL][tsIndex][iClk][id, rt, lr, pr, foundTime]
    multi_array<double, 4> stTsfFirmwareInfo{extents[4][15][48][5]};
    storeTSFirmwareData(stTsfFirmwareInfo);

    // stTsfInfo[stSL][iTS][id, rt, lr, pr, foundTime]
    multi_array<double, 3> stTsfInfo{extents[4][48][5]};

    filterTSData(stTsfFirmwareInfo, stTsfInfo);
    // Add to TS datastore
    addTSDatastore(stTsfInfo, 1);
  }

  if (m_add2DFinderToDatastore) {
    // Process firmware 2D

    // t2DFirmwareInfo[tIndex][iClk][valid, isOld, charge, rho, phi0]
    multi_array<double, 3> t2DFirmwareInfo{extents[4][48][5]};
    // t2DTsfFirmwareInfo[tIndex][iClk][axSL][id, rt, lr, pr]
    multi_array<double, 4> t2DTsfFirmwareInfo{extents[4][48][5][4]};
    store2DFirmwareData(t2DFirmwareInfo, t2DTsfFirmwareInfo);

    // t2DInfo[tIndex][charge, rho, phi0]
    multi_array<double, 2> t2DInfo{extents[0][3]};
    // t2DTsfInfo[tIndex][axSL][id, rt, lr, pr, -9999]
    multi_array<double, 3> t2DTsfInfo{extents[0][5][5]};

    filter2DData(t2DFirmwareInfo, t2DTsfFirmwareInfo, t2DInfo, t2DTsfInfo);
    add2DDatastore(t2DInfo, t2DTsfInfo);
  }

  if (m_addEventTimeToDatastore) {
    if (!m_eventTime.isValid()) m_eventTime.create();
    // add eventTime
    bool validEventTime = 0;
    int eventTime = 0;
    for (int iClk = 0; iClk < m_firmwareResults.getEntries() - 17 - 1; iClk++) {
      TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
      if (result->m_t3dtrk0_evtTimeValid_delay) {
        validEventTime = 1;
        eventTime = result->m_t3dtrk0_evtTime_delay;
      }
    }
    if (validEventTime) {
      m_eventTime->addBinnedEventT0(eventTime, Const::CDC);
    }
    if (m_isVerbose) {
      if (validEventTime) cout << "[ET] valid: 1 eventTime:" << eventTime << endl;
      else cout << "[ET] valid: 0" << endl;
    }
  }

  if (m_add3DToDatastore) {
    // t3DFirmwareInfo[tIndex][iClk][2DValid, 2DisOld, TSFValid, EventTimeValid, eventTime, charge, rho, phi0, z0, cot, zchi]
    multi_array<double, 3> t3DFirmwareInfo{extents[4][48][11]};

    // m_fit3DWithTSIM 0:firmware 1:fastSim 2:firmSim
    // Use Firmware results
    if (m_fit3DWithTSIM == 0) store3DFirmwareData(t3DFirmwareInfo);
    // Use fast sim with debug
    if (m_fit3DWithTSIM == 1) store3DFastSimData(t3DFirmwareInfo);
    // Use firm sim with debug
    if (m_fit3DWithTSIM == 2) store3DFirmSimData(t3DFirmwareInfo);

    // t3DInfo[eventTime, charge, rho, phi0, z0, cot, zchi]
    multi_array<double, 2> t3DInfo{extents[0][7]};


    filter3DData(t3DFirmwareInfo, t3DInfo);

    if (m_fit3DWithTSIM == 0) add3DDatastore(t3DInfo);
    else add3DDatastore(t3DInfo, 0);
  }

  if (m_isVerbose > 1) {
    debug3DFirmware();
  }



  //vector<int> t2DClks;
  //vector<int> t2DTrackId;
  //filter2DData(t2DFirmwareInfo, t2DTsfFirmwareInfo, t2DInfo, t2DTsfInfo, t2DClks, t2DTrackId);

  //// Combine common tracks
  //// 2D info
  //for(unsigned iTrack = 0; iTrack < t2DInfo.size(); iTrack++)
  //{
  //  cout<<"[2D] iClk: "<<t2DClks[iTrack]<<" trackid: "<<t2DTrackId[iTrack]<<" charge:"<<t2DInfo[iTrack][0]<<endl;
  //  int rho_s = t2DInfo[iTrack][1];
  //  int rho_2Dint = toSigned(rho_s, 7);
  //  double rho = 0.3*34/30/1.5e-4/abs(rho_2Dint);
  //  int rho_3Dint = rho*(pow(2,11)-0.5)/2500;
  //  cout<<"  rho_s "<<rho_s<<" rho_int: "<<rho_2Dint<<" rho: "<<rho<<" rho_3Dint: "<<rho_3Dint<<endl;;
  //  //cout<<"  phi0: "<<t2DInfo[iTrack][2]<<" s: "<<toSigned(t2DInfo[iTrack][2],13)<<" convert: "<<endl;
  //  //for(unsigned iAx = 0; iAx < t2DTsfInfo[iTrack].size(); iAx++)
  //  //{
  //  //  cout<<"  iAx: "<<iAx<<" id: "<<t2DTsfInfo[iTrack][iAx][0]<<" lr: "<<t2DTsfInfo[iTrack][iAx][2]<<endl;
  //  //}
  //}
  //// 3D info
  //for (unsigned iTrack = 0; iTrack < t3DInfo.size(); iTrack++)
  //{
  //  cout<<"iClk: "<<t3DClks[iTrack]<<" trackid: "<<t3DTrackId[iTrack]<<" validTS: "<<bitset<4>(t3DInfo[iTrack][0])<<" z0: "<<t3DInfo[iTrack][1]<<" cot: "<<t3DInfo[iTrack][2]<<" zchi: "<<t3DInfo[iTrack][3]<<endl;
  //}


}

int TRGCDCT3DConverterModule::toTSID(int iSL, int iWire)
{
  vector<int> nWires = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  int id = 0;
  for (int i = 0; i < (int)iSL; i++) {
    id += nWires[i];
  }

//Correct the segmentID.  The zero points of raw segmentID for four boards are different.
  if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore0") {
    id = id + iWire + nWires[(int)iSL] / 4.0 * 0;
    if (iWire + nWires[(int)iSL] / 4.0 * 0 > nWires[(int)iSL]) {
      id = id - nWires[(int)iSL];
    }
  } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore1") {
    id = id + iWire + nWires[(int)iSL] / 4.0 * 1;
    if (iWire + nWires[(int)iSL] / 4.0 * 1 > nWires[(int)iSL]) {
      id = id - nWires[(int)iSL];
    }
  } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore2") {
    id = id + iWire + nWires[(int)iSL] / 4.0 * 2;
    if (iWire + nWires[(int)iSL] / 4.0 * 2 > nWires[(int)iSL]) {
      id = id - nWires[(int)iSL];
    }
  } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore3") {
    id = id + iWire + nWires[(int)iSL] / 4.0 * 3;
    if (iWire + nWires[(int)iSL] / 4.0 * 3 > nWires[(int)iSL]) {
      id = id - nWires[(int)iSL];
    }
  }
  //cout<<"toID: iSL"<<iSL<<" iWire:"<<iWire<<" id:"<<id<<endl;
  return id;
}

int TRGCDCT3DConverterModule::toSigned(int value, int nBits)
{
  int signMask = 1 << (nBits - 1);
  if (value & signMask) {
    int mask = (1 << nBits) - 1;
    value |= ~mask;
  }
  return value;
}

////obselete
//int TRGCDCT3DConverterModule::t2DRhoTot3DRho(int value, bool isSigned)
//{
//  if (value == 0) return 2047;
//  if (value == 125) {
//    B2WARNING("Bug in 2D parser.");
//    return 451;
//  }
//  if (isSigned) return int(0.3 * 34 / 30 / 1.5e-4 / abs(toSigned(value, 7)) * (pow(2, 11) - 0.5) / 2500);
//  else return int(0.3 * 34 / 30 / 1.5e-4 / abs(value) * (pow(2, 11) - 0.5) / 2500);
//}
//

int TRGCDCT3DConverterModule::t2DPhiTot3DPhi(int phi, int rho)
{
  double phiReal = 45 + 90. / 80 * (1 + phi);
  if (toSigned(rho, 7) >= 0) phiReal -= 90;
  else phiReal += 90;

  while (phiReal > 180) {
    phiReal -= 360;
  }
  while (phiReal < -180) {
    phiReal += 360;
  }

  int phiInt = phiReal * (pow(2, 12) - 0.5) / 180;
  return phiInt;
}


void TRGCDCT3DConverterModule::filterTSData(multi_array<double, 4>& tsfFirmwareInfo,
                                            multi_array<double, 3>& tsfInfo)
{
  unsigned iTS_filter = 0;
  // iSl is stereo SL index
  for (unsigned iSL = 0; iSL < tsfFirmwareInfo.shape()[0]; iSL++) {
    iTS_filter = 0;
    for (unsigned iTS = 0; iTS < tsfFirmwareInfo.shape()[1]; iTS++) {
      for (unsigned iClk = 0; iClk < tsfFirmwareInfo.shape()[2]; iClk++) {
        double id = tsfFirmwareInfo[iSL][iTS][iClk][0];
        double rt = tsfFirmwareInfo[iSL][iTS][iClk][1];
        double lr = tsfFirmwareInfo[iSL][iTS][iClk][2];
        double pr = tsfFirmwareInfo[iSL][iTS][iClk][3];
        double ft = tsfFirmwareInfo[iSL][iTS][iClk][4];
        if (pr != 0) {
          double ts_ref[5] = {id, rt, lr, pr, ft};
          multi_array_ref<double, 1> ts((double*)ts_ref, extents[5]);
          tsfInfo[iSL][iTS_filter] = ts;
          iTS_filter = iTS_filter++;
        }
      }
    }
  }
}

//// t3DFirmwareInfo[tIndex][iClk][valid2D, validTS, z0, cot, zchi]
//// t3DInfo[tIndex][validTS, z0, cot, zchi]
//void TRGCDCT3DConverterModule::filter3DData(std::vector<std::vector<std::vector<double> > > & t3DFirmwareInfo, std::vector<std::vector<double> > & t3DInfo, std::vector<int> & t3DClks, std::vector<int> & t3DTrackId)
//{
//  for (unsigned iTrack = 0; iTrack < t3DFirmwareInfo.size(); iTrack++)
//  {
//    for (unsigned iClk = 0; iClk < t3DFirmwareInfo[iTrack].size(); iClk++)
//    {
//      double valid2D = t3DFirmwareInfo[iTrack][iClk][0];
//      double validTS = t3DFirmwareInfo[iTrack][iClk][1];
//      bitset<4> tsf_fnf(validTS);
//      int nStHits = int(tsf_fnf[0]) + int(tsf_fnf[1]) + int(tsf_fnf[2]) + int(tsf_fnf[3]);
//      if (valid2D != 1 || nStHits < 2) continue;
//      double z0 = t3DFirmwareInfo[iTrack][iClk][2];
//      double cot = t3DFirmwareInfo[iTrack][iClk][3];
//      double zchi = t3DFirmwareInfo[iTrack][iClk][4];
//      //cout<<"valid2D:"<<valid2D<<" tsf_fnf: "<<tsf_fnf<<" nSt: "<<nStHits<<endl;
//      vector<double> track = {validTS, z0, cot, zchi};
//      t3DInfo.push_back(track);
//      t3DClks.push_back(iClk);
//      t3DTrackId.push_back(iTrack);
//    }
//  }
//}


// t2DFirmwareInfo[tIndex][iClk][valid, isOld, charge, rho, phi0]
// t2DTsfFirmwareInfo[tIndex][iClk][axSL][id, rt, lr, pr]
// t2DInfo[tIndex][charge, rho, phi0]
// t2DTsfInfo[tIndex][axSL][id, rt, lr, pr, -9999]
void TRGCDCT3DConverterModule::filter2DData(multi_array<double, 3>& t2DFirmwareInfo,
                                            multi_array<double, 4>& t2DTsfFirmwareInfo, multi_array<double, 2>& t2DInfo,
                                            multi_array<double, 3>& t2DTsfInfo)
{

  unsigned tIndex_filter = 0;
  for (unsigned iTrack = 0; iTrack < t2DFirmwareInfo.shape()[0]; iTrack++) {
    for (unsigned iClk = 0; iClk < t2DFirmwareInfo.shape()[1]; iClk++) {
      if (t2DFirmwareInfo[iTrack][iClk][0] == 0) continue;
      // TODO make an algorithm to follow the track.

      //choose new tracks
      //if (t2DInfo.shape()[0] != 0 && t2DFirmwareInfo[iTrack][iClk][1] == 1) continue;

      double track_ref[3] = {t2DFirmwareInfo[iTrack][iClk][2], t2DFirmwareInfo[iTrack][iClk][3], t2DFirmwareInfo[iTrack][iClk][4]};
      multi_array_ref<double, 1> track((double*)track_ref, extents[3]);
      t2DInfo.resize(extents[tIndex_filter + 1][3]);
      t2DInfo[tIndex_filter] = track;


      multi_array<double, 2>  axTSInfo{extents[5][5]};
      for (unsigned iAx = 0; iAx < t2DTsfFirmwareInfo.shape()[2]; iAx++) {
        double id = t2DTsfFirmwareInfo[iTrack][iClk][iAx][0];
        double rt = t2DTsfFirmwareInfo[iTrack][iClk][iAx][1];
        double lr = t2DTsfFirmwareInfo[iTrack][iClk][iAx][2];
        double pr = t2DTsfFirmwareInfo[iTrack][iClk][iAx][3];

        axTSInfo[iAx][0] = id;
        axTSInfo[iAx][1] = rt;
        axTSInfo[iAx][2] = lr;
        axTSInfo[iAx][3] = pr;
        axTSInfo[iAx][4] = -9999;
      }
      t2DTsfInfo.resize(extents[tIndex_filter + 1][5][5]);
      t2DTsfInfo[tIndex_filter] = axTSInfo;
      tIndex_filter += 1;
    }
  }
  //for(unsigned iTrack = 0; iTrack < t2DInfo.size(); iTrack++)
  //{
  //  cout<<"iTrack: "<<iTrack<<" charge:"<<t2DInfo[iTrack][0]<<" rho: "<<t2DInfo[iTrack][1]<<" phi0: "<<t2DInfo[iTrack][2]<<endl;
  //  for(unsigned iAx = 0; iAx < t2DTsfInfo[iTrack].size(); iAx++)
  //  {
  //    cout<<"  iAx: "<<iAx<<" id: "<<t2DTsfInfo[iTrack][iAx][0]<<" lr: "<<t2DTsfInfo[iTrack][iAx][2]<<endl;
  //  }
  //}
}

//// t2DFirmwareInfo[tIndex][iClk][valid, charge, rho, phi0]
//// t2DTsfFirmwareInfo[tIndex][iClk][axSL][id, rt, lr, pr]
//// t2DInfo[tIndex][charge, rho, phi0]
//// t2DTsfInfo[tIndex][axSL][id, rt, lr, pr, -9999]
//void TRGCDCT3DConverterModule::filter2DData(std::vector<std::vector<std::vector<double> > > & t2DFirmwareInfo, std::vector<std::vector<std::vector<std::vector<double> > > > & t2DTsfFirmwareInfo, std::vector<std::vector<double> > & t2DInfo, std::vector<std::vector<std::vector<double> > > & t2DTsfInfo, std::vector<int> & t2DClks, std::vector<int> & t2DTrackId)
//{
//  for (unsigned iTrack = 0; iTrack < t2DFirmwareInfo.size(); iTrack++)
//  {
//    for (unsigned iClk = 0; iClk < t2DFirmwareInfo[iTrack].size(); iClk++)
//    {
//      if (t2DFirmwareInfo[iTrack][iClk][0] == 0) continue;
//
//      vector<double> track = {t2DFirmwareInfo[iTrack][iClk][1], t2DFirmwareInfo[iTrack][iClk][2], t2DFirmwareInfo[iTrack][iClk][3]};
//      t2DInfo.push_back(track);
//
//      vector<vector< double> > axTSInfo (5, vector<double> (5));
//      for (unsigned iAx = 0; iAx < t2DTsfFirmwareInfo[iTrack][iClk].size(); iAx++)
//      {
//        double id = t2DTsfFirmwareInfo[iTrack][iClk][iAx][0];
//        double rt = t2DTsfFirmwareInfo[iTrack][iClk][iAx][1];
//        double lr = t2DTsfFirmwareInfo[iTrack][iClk][iAx][2];
//        double pr = t2DTsfFirmwareInfo[iTrack][iClk][iAx][3];
//
//        axTSInfo[iAx][0] = id;
//        axTSInfo[iAx][1] = rt;
//        axTSInfo[iAx][2] = lr;
//        axTSInfo[iAx][3] = pr;
//        axTSInfo[iAx][4] = -9999;
//      }
//      t2DTsfInfo.push_back(axTSInfo);
//
//      t2DClks.push_back(iClk);
//      t2DTrackId.push_back(iTrack);
//    }
//  }
//  //for(unsigned iTrack = 0; iTrack < t2DInfo.size(); iTrack++)
//  //{
//  //  cout<<"iTrack: "<<iTrack<<" charge:"<<t2DInfo[iTrack][0]<<" rho: "<<t2DInfo[iTrack][1]<<" phi0: "<<t2DInfo[iTrack][2]<<endl;
//  //  for(unsigned iAx = 0; iAx < t2DTsfInfo[iTrack].size(); iAx++)
//  //  {
//  //    cout<<"  iAx: "<<iAx<<" id: "<<t2DTsfInfo[iTrack][iAx][0]<<" lr: "<<t2DTsfInfo[iTrack][iAx][2]<<endl;
//  //  }
//  //  cout<<"  iClk: "<<t2DClks[iTrack]<<" trackid: "<<t2DTrackId[iTrack]<<endl;
//  //}
//}

//// t2DInfo[tIndex][charge, rho, phi0]
//// t2DTsfInfo[tIndex][axSL][id, rt, lr, pr, -9999]
// t3DInfo[eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::add2DDatastore(multi_array<double, 2>& t2DInfo,
                                              multi_array<double, 3>& t2DTsfInfo)
{
  // Add 2D track
  for (unsigned iTrack = 0; iTrack < t2DInfo.shape()[0]; ++iTrack) {
    double charge = 0, phi0_i = 0, omega = 0, chi2D = 0;
    //double phi0_c = 0;
    //Convert
    charge = t2DInfo[iTrack][0] == 2 ? -1 : 1;
    //phi0_c = t2DPhiTot3DPhi(t2DInfo[iTrack][2], t2DInfo[iTrack][1]) / (pow(2, 12) - 0.5) * M_PI;
    //phi0_i = phi0_c + charge * M_PI_2;
    //cout<<phi0_i<<" "<<(45 + 90./80 * (1+ t2DInfo[iTrack][2]))/180*M_PI<<endl;
    phi0_i = (45 + 90. / 80 * (1 + t2DInfo[iTrack][2])) / 180 * M_PI;

    //Correct phi0_i.  The zero points of raw phi0_i for four boards are different.
    if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore0") {
      phi0_i = phi0_i + (M_PI / 2.0) * 0;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore1") {
      phi0_i = phi0_i + (M_PI / 2.0) * 1;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore2") {
      phi0_i = phi0_i + (M_PI / 2.0) * 2;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore3") {
      phi0_i = phi0_i + (M_PI / 2.0) * 3;
    }

    // Confine the range of the angle to -pi ~ pi
    while (phi0_i > M_PI) {
      phi0_i -= 2 * M_PI;
    }
    while (phi0_i < -M_PI) {
      phi0_i += 2 * M_PI;
    }

    //omega = charge / t2DRhoTot3DRho(t2DInfo[iTrack][1])/(pow(2,11)-0.5)*2500;
    //cout<<omega<<" "<<toSigned(t2DInfo[iTrack][1],7)*1.5e-4*30/0.3/34<<endl;
    //omega = toSigned(t2DInfo[iTrack][1], 7) * 1.5e-4 * 30 / 0.3 / 34;
    omega = toSigned(t2DInfo[iTrack][1], 7) / 33.0 * 3.2;
    //cout<<"iTrack: "<<iTrack<<" charge:"<<t2DInfo[iTrack][0]<<" "<<charge<<"  rho: "<<t2DInfo[iTrack][1]<<" "<<omega<<" phi0_i: "<<t2DInfo[iTrack][2]<<" "<<phi0_i<<endl;

    CDCTriggerTrack* track = m_tracks2D.appendNew(phi0_i, omega, chi2D, 0, 0, 0);

    if (m_isVerbose) cout << "[2D] iTrack:" << iTrack << " charge:" << charge << " phi0_i:" << phi0_i << " " << phi0_i * 180 / M_PI <<
                            " omega:" << omega << " pt:" << charge / omega * 0.3 * 1.5 * 0.01 << endl;

    for (unsigned iAx = 0; iAx < t2DTsfInfo.shape()[1]; ++iAx) {
      double rawId = t2DTsfInfo[iTrack][iAx][0];
      double id = -1;
      if (iAx != 4) id = rawId;
      else {
        id = rawId - 16;
        if (id < 0) id += 384;
      }
      double rt = t2DTsfInfo[iTrack][iAx][1];
      double lr = t2DTsfInfo[iTrack][iAx][2];
      double pr = t2DTsfInfo[iTrack][iAx][3];
      double ft = t2DTsfInfo[iTrack][iAx][4];
      if (pr == 0) continue;
      CDCHit prHit(rt, 0, iAx * 2, pr == 3 ? 2 : 3, id);
      m_hits.appendNew(prHit, toTSID(int(iAx * 2), id), pr, lr, rt, 0, ft);
      track->addRelationTo(m_hits[m_hits.getEntries() - 1]);
      if (m_isVerbose) cout << "[2D] iTrack: " << iTrack << " iAx:" << iAx << " id:" << id << " rt:" << rt << " lr:" << lr << " pr:" << pr
                              << " ft:" << ft << endl;
    }


  }
}

void TRGCDCT3DConverterModule::addTSDatastore(multi_array<double, 3>& tsfInfo, int isSt)
{
  for (unsigned iSL = 0; iSL < tsfInfo.shape()[0]; iSL++) {
    for (unsigned iTS = 0; iTS < tsfInfo.shape()[1]; iTS++) {
      double id = tsfInfo[iSL][iTS][0];
      double rt = tsfInfo[iSL][iTS][1];
      double lr = tsfInfo[iSL][iTS][2];
      double pr = tsfInfo[iSL][iTS][3];
      double ft = tsfInfo[iSL][iTS][4];
      if (pr != 0) {
        if (m_isVerbose) cout << "[TSF] iSL:" << iSL << " iTS:" << iTS << " id:" << id << " rt:" << rt << " lr:" << lr << " pr:" << pr <<
                                " ft:" << ft << endl;
        CDCHit prHit(rt, 0, iSL * 2 + isSt, pr == 3 ? 2 : 3, id);
        m_hits.appendNew(prHit, toTSID(int(iSL * 2 + isSt), id), pr, lr, rt, 0, ft);
      }
    }
  }
}

void TRGCDCT3DConverterModule::storeTSFirmwareData(multi_array<double, 4>& tsfInfo)
{
  for (int iClk = 0; iClk < m_firmwareResults.getEntries(); iClk++) {
    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    //cout<<"iClk:"<<iClk<<" tsf1_cc:"<<result->m_tsf1_cc<<" tsf1ts0_id:"<<result->m_tsf1ts0_id<<" tsf1ts0_rt:"<<result->m_tsf1ts0_rt<<" tsf1ts0_lr:"<<result->m_tsf1ts0_lr<<" tsf1ts0_pr:"<<result->m_tsf1ts0_pr<<endl;
    tsfInfo[0][0][iClk][0] = result->m_tsf1ts0_id;
    tsfInfo[0][0][iClk][1] = result->m_tsf1ts0_rt;
    tsfInfo[0][0][iClk][2] = result->m_tsf1ts0_lr;
    tsfInfo[0][0][iClk][3] = result->m_tsf1ts0_pr;
    tsfInfo[0][0][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][1][iClk][0] = result->m_tsf1ts1_id;
    tsfInfo[0][1][iClk][1] = result->m_tsf1ts1_rt;
    tsfInfo[0][1][iClk][2] = result->m_tsf1ts1_lr;
    tsfInfo[0][1][iClk][3] = result->m_tsf1ts1_pr;
    tsfInfo[0][1][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][2][iClk][0] = result->m_tsf1ts2_id;
    tsfInfo[0][2][iClk][1] = result->m_tsf1ts2_rt;
    tsfInfo[0][2][iClk][2] = result->m_tsf1ts2_lr;
    tsfInfo[0][2][iClk][3] = result->m_tsf1ts2_pr;
    tsfInfo[0][2][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][3][iClk][0] = result->m_tsf1ts3_id;
    tsfInfo[0][3][iClk][1] = result->m_tsf1ts3_rt;
    tsfInfo[0][3][iClk][2] = result->m_tsf1ts3_lr;
    tsfInfo[0][3][iClk][3] = result->m_tsf1ts3_pr;
    tsfInfo[0][3][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][4][iClk][0] = result->m_tsf1ts4_id;
    tsfInfo[0][4][iClk][1] = result->m_tsf1ts4_rt;
    tsfInfo[0][4][iClk][2] = result->m_tsf1ts4_lr;
    tsfInfo[0][4][iClk][3] = result->m_tsf1ts4_pr;
    tsfInfo[0][4][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][5][iClk][0] = result->m_tsf1ts5_id;
    tsfInfo[0][5][iClk][1] = result->m_tsf1ts5_rt;
    tsfInfo[0][5][iClk][2] = result->m_tsf1ts5_lr;
    tsfInfo[0][5][iClk][3] = result->m_tsf1ts5_pr;
    tsfInfo[0][5][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][6][iClk][0] = result->m_tsf1ts6_id;
    tsfInfo[0][6][iClk][1] = result->m_tsf1ts6_rt;
    tsfInfo[0][6][iClk][2] = result->m_tsf1ts6_lr;
    tsfInfo[0][6][iClk][3] = result->m_tsf1ts6_pr;
    tsfInfo[0][6][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][7][iClk][0] = result->m_tsf1ts7_id;
    tsfInfo[0][7][iClk][1] = result->m_tsf1ts7_rt;
    tsfInfo[0][7][iClk][2] = result->m_tsf1ts7_lr;
    tsfInfo[0][7][iClk][3] = result->m_tsf1ts7_pr;
    tsfInfo[0][7][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][8][iClk][0] = result->m_tsf1ts8_id;
    tsfInfo[0][8][iClk][1] = result->m_tsf1ts8_rt;
    tsfInfo[0][8][iClk][2] = result->m_tsf1ts8_lr;
    tsfInfo[0][8][iClk][3] = result->m_tsf1ts8_pr;
    tsfInfo[0][8][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][9][iClk][0] = result->m_tsf1ts9_id;
    tsfInfo[0][9][iClk][1] = result->m_tsf1ts9_rt;
    tsfInfo[0][9][iClk][2] = result->m_tsf1ts9_lr;
    tsfInfo[0][9][iClk][3] = result->m_tsf1ts9_pr;
    tsfInfo[0][9][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][10][iClk][0] = result->m_tsf1ts10_id;
    tsfInfo[0][10][iClk][1] = result->m_tsf1ts10_rt;
    tsfInfo[0][10][iClk][2] = result->m_tsf1ts10_lr;
    tsfInfo[0][10][iClk][3] = result->m_tsf1ts10_pr;
    tsfInfo[0][10][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][11][iClk][0] = result->m_tsf1ts11_id;
    tsfInfo[0][11][iClk][1] = result->m_tsf1ts11_rt;
    tsfInfo[0][11][iClk][2] = result->m_tsf1ts11_lr;
    tsfInfo[0][11][iClk][3] = result->m_tsf1ts11_pr;
    tsfInfo[0][11][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][12][iClk][0] = result->m_tsf1ts12_id;
    tsfInfo[0][12][iClk][1] = result->m_tsf1ts12_rt;
    tsfInfo[0][12][iClk][2] = result->m_tsf1ts12_lr;
    tsfInfo[0][12][iClk][3] = result->m_tsf1ts12_pr;
    tsfInfo[0][12][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][13][iClk][0] = result->m_tsf1ts13_id;
    tsfInfo[0][13][iClk][1] = result->m_tsf1ts13_rt;
    tsfInfo[0][13][iClk][2] = result->m_tsf1ts13_lr;
    tsfInfo[0][13][iClk][3] = result->m_tsf1ts13_pr;
    tsfInfo[0][13][iClk][4] = result->m_tsf1_cc;
    tsfInfo[0][14][iClk][0] = result->m_tsf1ts14_id;
    tsfInfo[0][14][iClk][1] = result->m_tsf1ts14_rt;
    tsfInfo[0][14][iClk][2] = result->m_tsf1ts14_lr;
    tsfInfo[0][14][iClk][3] = result->m_tsf1ts14_pr;
    tsfInfo[0][14][iClk][4] = result->m_tsf1_cc;

    tsfInfo[1][0][iClk][0] = result->m_tsf3ts0_id;
    tsfInfo[1][0][iClk][1] = result->m_tsf3ts0_rt;
    tsfInfo[1][0][iClk][2] = result->m_tsf3ts0_lr;
    tsfInfo[1][0][iClk][3] = result->m_tsf3ts0_pr;
    tsfInfo[1][0][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][1][iClk][0] = result->m_tsf3ts1_id;
    tsfInfo[1][1][iClk][1] = result->m_tsf3ts1_rt;
    tsfInfo[1][1][iClk][2] = result->m_tsf3ts1_lr;
    tsfInfo[1][1][iClk][3] = result->m_tsf3ts1_pr;
    tsfInfo[1][1][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][2][iClk][0] = result->m_tsf3ts2_id;
    tsfInfo[1][2][iClk][1] = result->m_tsf3ts2_rt;
    tsfInfo[1][2][iClk][2] = result->m_tsf3ts2_lr;
    tsfInfo[1][2][iClk][3] = result->m_tsf3ts2_pr;
    tsfInfo[1][2][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][3][iClk][0] = result->m_tsf3ts3_id;
    tsfInfo[1][3][iClk][1] = result->m_tsf3ts3_rt;
    tsfInfo[1][3][iClk][2] = result->m_tsf3ts3_lr;
    tsfInfo[1][3][iClk][3] = result->m_tsf3ts3_pr;
    tsfInfo[1][3][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][4][iClk][0] = result->m_tsf3ts4_id;
    tsfInfo[1][4][iClk][1] = result->m_tsf3ts4_rt;
    tsfInfo[1][4][iClk][2] = result->m_tsf3ts4_lr;
    tsfInfo[1][4][iClk][3] = result->m_tsf3ts4_pr;
    tsfInfo[1][4][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][5][iClk][0] = result->m_tsf3ts5_id;
    tsfInfo[1][5][iClk][1] = result->m_tsf3ts5_rt;
    tsfInfo[1][5][iClk][2] = result->m_tsf3ts5_lr;
    tsfInfo[1][5][iClk][3] = result->m_tsf3ts5_pr;
    tsfInfo[1][5][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][6][iClk][0] = result->m_tsf3ts6_id;
    tsfInfo[1][6][iClk][1] = result->m_tsf3ts6_rt;
    tsfInfo[1][6][iClk][2] = result->m_tsf3ts6_lr;
    tsfInfo[1][6][iClk][3] = result->m_tsf3ts6_pr;
    tsfInfo[1][6][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][7][iClk][0] = result->m_tsf3ts7_id;
    tsfInfo[1][7][iClk][1] = result->m_tsf3ts7_rt;
    tsfInfo[1][7][iClk][2] = result->m_tsf3ts7_lr;
    tsfInfo[1][7][iClk][3] = result->m_tsf3ts7_pr;
    tsfInfo[1][7][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][8][iClk][0] = result->m_tsf3ts8_id;
    tsfInfo[1][8][iClk][1] = result->m_tsf3ts8_rt;
    tsfInfo[1][8][iClk][2] = result->m_tsf3ts8_lr;
    tsfInfo[1][8][iClk][3] = result->m_tsf3ts8_pr;
    tsfInfo[1][8][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][9][iClk][0] = result->m_tsf3ts9_id;
    tsfInfo[1][9][iClk][1] = result->m_tsf3ts9_rt;
    tsfInfo[1][9][iClk][2] = result->m_tsf3ts9_lr;
    tsfInfo[1][9][iClk][3] = result->m_tsf3ts9_pr;
    tsfInfo[1][9][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][10][iClk][0] = result->m_tsf3ts10_id;
    tsfInfo[1][10][iClk][1] = result->m_tsf3ts10_rt;
    tsfInfo[1][10][iClk][2] = result->m_tsf3ts10_lr;
    tsfInfo[1][10][iClk][3] = result->m_tsf3ts10_pr;
    tsfInfo[1][10][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][11][iClk][0] = result->m_tsf3ts11_id;
    tsfInfo[1][11][iClk][1] = result->m_tsf3ts11_rt;
    tsfInfo[1][11][iClk][2] = result->m_tsf3ts11_lr;
    tsfInfo[1][11][iClk][3] = result->m_tsf3ts11_pr;
    tsfInfo[1][11][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][12][iClk][0] = result->m_tsf3ts12_id;
    tsfInfo[1][12][iClk][1] = result->m_tsf3ts12_rt;
    tsfInfo[1][12][iClk][2] = result->m_tsf3ts12_lr;
    tsfInfo[1][12][iClk][3] = result->m_tsf3ts12_pr;
    tsfInfo[1][12][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][13][iClk][0] = result->m_tsf3ts13_id;
    tsfInfo[1][13][iClk][1] = result->m_tsf3ts13_rt;
    tsfInfo[1][13][iClk][2] = result->m_tsf3ts13_lr;
    tsfInfo[1][13][iClk][3] = result->m_tsf3ts13_pr;
    tsfInfo[1][13][iClk][4] = result->m_tsf3_cc;
    tsfInfo[1][14][iClk][0] = result->m_tsf3ts14_id;
    tsfInfo[1][14][iClk][1] = result->m_tsf3ts14_rt;
    tsfInfo[1][14][iClk][2] = result->m_tsf3ts14_lr;
    tsfInfo[1][14][iClk][3] = result->m_tsf3ts14_pr;
    tsfInfo[1][14][iClk][4] = result->m_tsf3_cc;

    tsfInfo[2][0][iClk][0] = result->m_tsf5ts0_id;
    tsfInfo[2][0][iClk][1] = result->m_tsf5ts0_rt;
    tsfInfo[2][0][iClk][2] = result->m_tsf5ts0_lr;
    tsfInfo[2][0][iClk][3] = result->m_tsf5ts0_pr;
    tsfInfo[2][0][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][1][iClk][0] = result->m_tsf5ts1_id;
    tsfInfo[2][1][iClk][1] = result->m_tsf5ts1_rt;
    tsfInfo[2][1][iClk][2] = result->m_tsf5ts1_lr;
    tsfInfo[2][1][iClk][3] = result->m_tsf5ts1_pr;
    tsfInfo[2][1][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][2][iClk][0] = result->m_tsf5ts2_id;
    tsfInfo[2][2][iClk][1] = result->m_tsf5ts2_rt;
    tsfInfo[2][2][iClk][2] = result->m_tsf5ts2_lr;
    tsfInfo[2][2][iClk][3] = result->m_tsf5ts2_pr;
    tsfInfo[2][2][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][3][iClk][0] = result->m_tsf5ts3_id;
    tsfInfo[2][3][iClk][1] = result->m_tsf5ts3_rt;
    tsfInfo[2][3][iClk][2] = result->m_tsf5ts3_lr;
    tsfInfo[2][3][iClk][3] = result->m_tsf5ts3_pr;
    tsfInfo[2][3][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][4][iClk][0] = result->m_tsf5ts4_id;
    tsfInfo[2][4][iClk][1] = result->m_tsf5ts4_rt;
    tsfInfo[2][4][iClk][2] = result->m_tsf5ts4_lr;
    tsfInfo[2][4][iClk][3] = result->m_tsf5ts4_pr;
    tsfInfo[2][4][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][5][iClk][0] = result->m_tsf5ts5_id;
    tsfInfo[2][5][iClk][1] = result->m_tsf5ts5_rt;
    tsfInfo[2][5][iClk][2] = result->m_tsf5ts5_lr;
    tsfInfo[2][5][iClk][3] = result->m_tsf5ts5_pr;
    tsfInfo[2][5][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][6][iClk][0] = result->m_tsf5ts6_id;
    tsfInfo[2][6][iClk][1] = result->m_tsf5ts6_rt;
    tsfInfo[2][6][iClk][2] = result->m_tsf5ts6_lr;
    tsfInfo[2][6][iClk][3] = result->m_tsf5ts6_pr;
    tsfInfo[2][6][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][7][iClk][0] = result->m_tsf5ts7_id;
    tsfInfo[2][7][iClk][1] = result->m_tsf5ts7_rt;
    tsfInfo[2][7][iClk][2] = result->m_tsf5ts7_lr;
    tsfInfo[2][7][iClk][3] = result->m_tsf5ts7_pr;
    tsfInfo[2][7][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][8][iClk][0] = result->m_tsf5ts8_id;
    tsfInfo[2][8][iClk][1] = result->m_tsf5ts8_rt;
    tsfInfo[2][8][iClk][2] = result->m_tsf5ts8_lr;
    tsfInfo[2][8][iClk][3] = result->m_tsf5ts8_pr;
    tsfInfo[2][8][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][9][iClk][0] = result->m_tsf5ts9_id;
    tsfInfo[2][9][iClk][1] = result->m_tsf5ts9_rt;
    tsfInfo[2][9][iClk][2] = result->m_tsf5ts9_lr;
    tsfInfo[2][9][iClk][3] = result->m_tsf5ts9_pr;
    tsfInfo[2][9][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][10][iClk][0] = result->m_tsf5ts10_id;
    tsfInfo[2][10][iClk][1] = result->m_tsf5ts10_rt;
    tsfInfo[2][10][iClk][2] = result->m_tsf5ts10_lr;
    tsfInfo[2][10][iClk][3] = result->m_tsf5ts10_pr;
    tsfInfo[2][10][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][11][iClk][0] = result->m_tsf5ts11_id;
    tsfInfo[2][11][iClk][1] = result->m_tsf5ts11_rt;
    tsfInfo[2][11][iClk][2] = result->m_tsf5ts11_lr;
    tsfInfo[2][11][iClk][3] = result->m_tsf5ts11_pr;
    tsfInfo[2][11][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][12][iClk][0] = result->m_tsf5ts12_id;
    tsfInfo[2][12][iClk][1] = result->m_tsf5ts12_rt;
    tsfInfo[2][12][iClk][2] = result->m_tsf5ts12_lr;
    tsfInfo[2][12][iClk][3] = result->m_tsf5ts12_pr;
    tsfInfo[2][12][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][13][iClk][0] = result->m_tsf5ts13_id;
    tsfInfo[2][13][iClk][1] = result->m_tsf5ts13_rt;
    tsfInfo[2][13][iClk][2] = result->m_tsf5ts13_lr;
    tsfInfo[2][13][iClk][3] = result->m_tsf5ts13_pr;
    tsfInfo[2][13][iClk][4] = result->m_tsf5_cc;
    tsfInfo[2][14][iClk][0] = result->m_tsf5ts14_id;
    tsfInfo[2][14][iClk][1] = result->m_tsf5ts14_rt;
    tsfInfo[2][14][iClk][2] = result->m_tsf5ts14_lr;
    tsfInfo[2][14][iClk][3] = result->m_tsf5ts14_pr;
    tsfInfo[2][14][iClk][4] = result->m_tsf5_cc;

    tsfInfo[3][0][iClk][0] = result->m_tsf7ts0_id;
    tsfInfo[3][0][iClk][1] = result->m_tsf7ts0_rt;
    tsfInfo[3][0][iClk][2] = result->m_tsf7ts0_lr;
    tsfInfo[3][0][iClk][3] = result->m_tsf7ts0_pr;
    tsfInfo[3][0][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][1][iClk][0] = result->m_tsf7ts1_id;
    tsfInfo[3][1][iClk][1] = result->m_tsf7ts1_rt;
    tsfInfo[3][1][iClk][2] = result->m_tsf7ts1_lr;
    tsfInfo[3][1][iClk][3] = result->m_tsf7ts1_pr;
    tsfInfo[3][1][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][2][iClk][0] = result->m_tsf7ts2_id;
    tsfInfo[3][2][iClk][1] = result->m_tsf7ts2_rt;
    tsfInfo[3][2][iClk][2] = result->m_tsf7ts2_lr;
    tsfInfo[3][2][iClk][3] = result->m_tsf7ts2_pr;
    tsfInfo[3][2][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][3][iClk][0] = result->m_tsf7ts3_id;
    tsfInfo[3][3][iClk][1] = result->m_tsf7ts3_rt;
    tsfInfo[3][3][iClk][2] = result->m_tsf7ts3_lr;
    tsfInfo[3][3][iClk][3] = result->m_tsf7ts3_pr;
    tsfInfo[3][3][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][4][iClk][0] = result->m_tsf7ts4_id;
    tsfInfo[3][4][iClk][1] = result->m_tsf7ts4_rt;
    tsfInfo[3][4][iClk][2] = result->m_tsf7ts4_lr;
    tsfInfo[3][4][iClk][3] = result->m_tsf7ts4_pr;
    tsfInfo[3][4][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][5][iClk][0] = result->m_tsf7ts5_id;
    tsfInfo[3][5][iClk][1] = result->m_tsf7ts5_rt;
    tsfInfo[3][5][iClk][2] = result->m_tsf7ts5_lr;
    tsfInfo[3][5][iClk][3] = result->m_tsf7ts5_pr;
    tsfInfo[3][5][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][6][iClk][0] = result->m_tsf7ts6_id;
    tsfInfo[3][6][iClk][1] = result->m_tsf7ts6_rt;
    tsfInfo[3][6][iClk][2] = result->m_tsf7ts6_lr;
    tsfInfo[3][6][iClk][3] = result->m_tsf7ts6_pr;
    tsfInfo[3][6][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][7][iClk][0] = result->m_tsf7ts7_id;
    tsfInfo[3][7][iClk][1] = result->m_tsf7ts7_rt;
    tsfInfo[3][7][iClk][2] = result->m_tsf7ts7_lr;
    tsfInfo[3][7][iClk][3] = result->m_tsf7ts7_pr;
    tsfInfo[3][7][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][8][iClk][0] = result->m_tsf7ts8_id;
    tsfInfo[3][8][iClk][1] = result->m_tsf7ts8_rt;
    tsfInfo[3][8][iClk][2] = result->m_tsf7ts8_lr;
    tsfInfo[3][8][iClk][3] = result->m_tsf7ts8_pr;
    tsfInfo[3][8][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][9][iClk][0] = result->m_tsf7ts9_id;
    tsfInfo[3][9][iClk][1] = result->m_tsf7ts9_rt;
    tsfInfo[3][9][iClk][2] = result->m_tsf7ts9_lr;
    tsfInfo[3][9][iClk][3] = result->m_tsf7ts9_pr;
    tsfInfo[3][9][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][10][iClk][0] = result->m_tsf7ts10_id;
    tsfInfo[3][10][iClk][1] = result->m_tsf7ts10_rt;
    tsfInfo[3][10][iClk][2] = result->m_tsf7ts10_lr;
    tsfInfo[3][10][iClk][3] = result->m_tsf7ts10_pr;
    tsfInfo[3][10][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][11][iClk][0] = result->m_tsf7ts11_id;
    tsfInfo[3][11][iClk][1] = result->m_tsf7ts11_rt;
    tsfInfo[3][11][iClk][2] = result->m_tsf7ts11_lr;
    tsfInfo[3][11][iClk][3] = result->m_tsf7ts11_pr;
    tsfInfo[3][11][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][12][iClk][0] = result->m_tsf7ts12_id;
    tsfInfo[3][12][iClk][1] = result->m_tsf7ts12_rt;
    tsfInfo[3][12][iClk][2] = result->m_tsf7ts12_lr;
    tsfInfo[3][12][iClk][3] = result->m_tsf7ts12_pr;
    tsfInfo[3][12][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][13][iClk][0] = result->m_tsf7ts13_id;
    tsfInfo[3][13][iClk][1] = result->m_tsf7ts13_rt;
    tsfInfo[3][13][iClk][2] = result->m_tsf7ts13_lr;
    tsfInfo[3][13][iClk][3] = result->m_tsf7ts13_pr;
    tsfInfo[3][13][iClk][4] = result->m_tsf7_cc;
    tsfInfo[3][14][iClk][0] = result->m_tsf7ts14_id;
    tsfInfo[3][14][iClk][1] = result->m_tsf7ts14_rt;
    tsfInfo[3][14][iClk][2] = result->m_tsf7ts14_lr;
    tsfInfo[3][14][iClk][3] = result->m_tsf7ts14_pr;
    tsfInfo[3][14][iClk][4] = result->m_tsf7_cc;

  }
}

// t2DFirmwareInfo[tIndex][iClk][valid, isOld charge, rho, phi0]
// t2DTsfFirmwareInfo[tIndex][iClk][axSL][id, rt, lr, pr]
void TRGCDCT3DConverterModule::store2DFirmwareData(multi_array<double, 3>& t2DFirmwareInfo,
                                                   multi_array<double, 4>& t2DTsfFirmwareInfo)
{

  for (int iClk = 0; iClk < m_firmwareResults.getEntries(); iClk++) {
    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    bitset<6> t2d_fnf(int(result->m_t2d_fnf));
    bitset<6> t2d_oldfnf(int(result->m_t3d_2doldtrk));
    //cout<<"iclk:"<<iClk<<" fnf:"<<result->m_t2d_fnf<<" "<<t2d_fnf<<" "<<t2d_fnf[5]<<endl;
    //cout<<"iClk:"<<iClk<<" t2d_fnf:"<<m_firmwareResults[iClk]->m_t2d_fnf<<endl;

    t2DFirmwareInfo[0][iClk][0] = t2d_fnf[5];
    t2DFirmwareInfo[0][iClk][1] = t2d_oldfnf[5];;
    t2DFirmwareInfo[0][iClk][2] = result->m_t2d0_charge;
    t2DFirmwareInfo[0][iClk][3] = result->m_t2d0_rho_s;
    t2DFirmwareInfo[0][iClk][4] = result->m_t2d0_phi;

    t2DFirmwareInfo[1][iClk][0] = t2d_fnf[4];
    t2DFirmwareInfo[1][iClk][1] = t2d_oldfnf[4];;
    t2DFirmwareInfo[1][iClk][2] = result->m_t2d1_charge;
    t2DFirmwareInfo[1][iClk][3] = result->m_t2d1_rho_s;
    t2DFirmwareInfo[1][iClk][4] = result->m_t2d1_phi;

    t2DFirmwareInfo[2][iClk][0] = t2d_fnf[3];
    t2DFirmwareInfo[2][iClk][1] = t2d_oldfnf[3];;
    t2DFirmwareInfo[2][iClk][2] = result->m_t2d2_charge;
    t2DFirmwareInfo[2][iClk][3] = result->m_t2d2_rho_s;
    t2DFirmwareInfo[2][iClk][4] = result->m_t2d2_phi;

    t2DFirmwareInfo[3][iClk][0] = t2d_fnf[2];
    t2DFirmwareInfo[3][iClk][1] = t2d_oldfnf[2];;
    t2DFirmwareInfo[3][iClk][2] = result->m_t2d3_charge;
    t2DFirmwareInfo[3][iClk][3] = result->m_t2d3_rho_s;
    t2DFirmwareInfo[3][iClk][4] = result->m_t2d3_phi;

    t2DTsfFirmwareInfo[0][iClk][0][0] = result->m_t2d0ts0_id;
    t2DTsfFirmwareInfo[0][iClk][0][1] = result->m_t2d0ts0_rt;
    t2DTsfFirmwareInfo[0][iClk][0][2] = result->m_t2d0ts0_lr;
    t2DTsfFirmwareInfo[0][iClk][0][3] = result->m_t2d0ts0_pr;
    t2DTsfFirmwareInfo[0][iClk][1][0] = result->m_t2d0ts2_id;
    t2DTsfFirmwareInfo[0][iClk][1][1] = result->m_t2d0ts2_rt;
    t2DTsfFirmwareInfo[0][iClk][1][2] = result->m_t2d0ts2_lr;
    t2DTsfFirmwareInfo[0][iClk][1][3] = result->m_t2d0ts2_pr;
    t2DTsfFirmwareInfo[0][iClk][2][0] = result->m_t2d0ts4_id;
    t2DTsfFirmwareInfo[0][iClk][2][1] = result->m_t2d0ts4_rt;
    t2DTsfFirmwareInfo[0][iClk][2][2] = result->m_t2d0ts4_lr;
    t2DTsfFirmwareInfo[0][iClk][2][3] = result->m_t2d0ts4_pr;
    t2DTsfFirmwareInfo[0][iClk][3][0] = result->m_t2d0ts6_id;
    t2DTsfFirmwareInfo[0][iClk][3][1] = result->m_t2d0ts6_rt;
    t2DTsfFirmwareInfo[0][iClk][3][2] = result->m_t2d0ts6_lr;
    t2DTsfFirmwareInfo[0][iClk][3][3] = result->m_t2d0ts6_pr;
    t2DTsfFirmwareInfo[0][iClk][4][0] = result->m_t2d0ts8_id;
    t2DTsfFirmwareInfo[0][iClk][4][1] = result->m_t2d0ts8_rt;
    t2DTsfFirmwareInfo[0][iClk][4][2] = result->m_t2d0ts8_lr;
    t2DTsfFirmwareInfo[0][iClk][4][3] = result->m_t2d0ts8_pr;

    t2DTsfFirmwareInfo[1][iClk][0][0] = result->m_t2d1ts0_id;
    t2DTsfFirmwareInfo[1][iClk][0][1] = result->m_t2d1ts0_rt;
    t2DTsfFirmwareInfo[1][iClk][0][2] = result->m_t2d1ts0_lr;
    t2DTsfFirmwareInfo[1][iClk][0][3] = result->m_t2d1ts0_pr;
    t2DTsfFirmwareInfo[1][iClk][1][0] = result->m_t2d1ts2_id;
    t2DTsfFirmwareInfo[1][iClk][1][1] = result->m_t2d1ts2_rt;
    t2DTsfFirmwareInfo[1][iClk][1][2] = result->m_t2d1ts2_lr;
    t2DTsfFirmwareInfo[1][iClk][1][3] = result->m_t2d1ts2_pr;
    t2DTsfFirmwareInfo[1][iClk][2][0] = result->m_t2d1ts4_id;
    t2DTsfFirmwareInfo[1][iClk][2][1] = result->m_t2d1ts4_rt;
    t2DTsfFirmwareInfo[1][iClk][2][2] = result->m_t2d1ts4_lr;
    t2DTsfFirmwareInfo[1][iClk][2][3] = result->m_t2d1ts4_pr;
    t2DTsfFirmwareInfo[1][iClk][3][0] = result->m_t2d1ts6_id;
    t2DTsfFirmwareInfo[1][iClk][3][1] = result->m_t2d1ts6_rt;
    t2DTsfFirmwareInfo[1][iClk][3][2] = result->m_t2d1ts6_lr;
    t2DTsfFirmwareInfo[1][iClk][3][3] = result->m_t2d1ts6_pr;
    t2DTsfFirmwareInfo[1][iClk][4][0] = result->m_t2d1ts8_id;
    t2DTsfFirmwareInfo[1][iClk][4][1] = result->m_t2d1ts8_rt;
    t2DTsfFirmwareInfo[1][iClk][4][2] = result->m_t2d1ts8_lr;
    t2DTsfFirmwareInfo[1][iClk][4][3] = result->m_t2d1ts8_pr;

    t2DTsfFirmwareInfo[2][iClk][0][0] = result->m_t2d2ts0_id;
    t2DTsfFirmwareInfo[2][iClk][0][1] = result->m_t2d2ts0_rt;
    t2DTsfFirmwareInfo[2][iClk][0][2] = result->m_t2d2ts0_lr;
    t2DTsfFirmwareInfo[2][iClk][0][3] = result->m_t2d2ts0_pr;
    t2DTsfFirmwareInfo[2][iClk][1][0] = result->m_t2d2ts2_id;
    t2DTsfFirmwareInfo[2][iClk][1][1] = result->m_t2d2ts2_rt;
    t2DTsfFirmwareInfo[2][iClk][1][2] = result->m_t2d2ts2_lr;
    t2DTsfFirmwareInfo[2][iClk][1][3] = result->m_t2d2ts2_pr;
    t2DTsfFirmwareInfo[2][iClk][2][0] = result->m_t2d2ts4_id;
    t2DTsfFirmwareInfo[2][iClk][2][1] = result->m_t2d2ts4_rt;
    t2DTsfFirmwareInfo[2][iClk][2][2] = result->m_t2d2ts4_lr;
    t2DTsfFirmwareInfo[2][iClk][2][3] = result->m_t2d2ts4_pr;
    t2DTsfFirmwareInfo[2][iClk][3][0] = result->m_t2d2ts6_id;
    t2DTsfFirmwareInfo[2][iClk][3][1] = result->m_t2d2ts6_rt;
    t2DTsfFirmwareInfo[2][iClk][3][2] = result->m_t2d2ts6_lr;
    t2DTsfFirmwareInfo[2][iClk][3][3] = result->m_t2d2ts6_pr;
    t2DTsfFirmwareInfo[2][iClk][4][0] = result->m_t2d2ts8_id;
    t2DTsfFirmwareInfo[2][iClk][4][1] = result->m_t2d2ts8_rt;
    t2DTsfFirmwareInfo[2][iClk][4][2] = result->m_t2d2ts8_lr;
    t2DTsfFirmwareInfo[2][iClk][4][3] = result->m_t2d2ts8_pr;

    t2DTsfFirmwareInfo[3][iClk][0][0] = result->m_t2d3ts0_id;
    t2DTsfFirmwareInfo[3][iClk][0][1] = result->m_t2d3ts0_rt;
    t2DTsfFirmwareInfo[3][iClk][0][2] = result->m_t2d3ts0_lr;
    t2DTsfFirmwareInfo[3][iClk][0][3] = result->m_t2d3ts0_pr;
    t2DTsfFirmwareInfo[3][iClk][1][0] = result->m_t2d3ts2_id;
    t2DTsfFirmwareInfo[3][iClk][1][1] = result->m_t2d3ts2_rt;
    t2DTsfFirmwareInfo[3][iClk][1][2] = result->m_t2d3ts2_lr;
    t2DTsfFirmwareInfo[3][iClk][1][3] = result->m_t2d3ts2_pr;
    t2DTsfFirmwareInfo[3][iClk][2][0] = result->m_t2d3ts4_id;
    t2DTsfFirmwareInfo[3][iClk][2][1] = result->m_t2d3ts4_rt;
    t2DTsfFirmwareInfo[3][iClk][2][2] = result->m_t2d3ts4_lr;
    t2DTsfFirmwareInfo[3][iClk][2][3] = result->m_t2d3ts4_pr;
    t2DTsfFirmwareInfo[3][iClk][3][0] = result->m_t2d3ts6_id;
    t2DTsfFirmwareInfo[3][iClk][3][1] = result->m_t2d3ts6_rt;
    t2DTsfFirmwareInfo[3][iClk][3][2] = result->m_t2d3ts6_lr;
    t2DTsfFirmwareInfo[3][iClk][3][3] = result->m_t2d3ts6_pr;
    t2DTsfFirmwareInfo[3][iClk][4][0] = result->m_t2d3ts8_id;
    t2DTsfFirmwareInfo[3][iClk][4][1] = result->m_t2d3ts8_rt;
    t2DTsfFirmwareInfo[3][iClk][4][2] = result->m_t2d3ts8_lr;
    t2DTsfFirmwareInfo[3][iClk][4][3] = result->m_t2d3ts8_pr;
  }
}


// t3DFirmwareInfo[tIndex][iClk][2DValid, 2DisOld, TSFValid, EventTimeValid, eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::store3DFirmwareData(multi_array<double, 3>& t3DFirmwareInfo)
{

  for (int iClk = 0; iClk < m_firmwareResults.getEntries() - 17 - 1; iClk++) {
    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    TRGCDCT3DUnpackerStore* result3D = m_firmwareResults[iClk + 17];

    bitset<6> t3d_fnf(int(result3D->m_t3d_2dfnf));
    bitset<4> tsf_fnf(int(result3D->m_t3d_validTS));
    bitset<6> t3d_oldfnf(int(result->m_t3d_2doldtrk));

    //cout<<iClk<<" "<<t3d_fnf<<" "<<t3d_oldfnf<<endl;

//The evtTime information was for debugging, but the currnet firmware doese not output the evtTime information time corresponding to every the track.

    t3DFirmwareInfo[0][iClk][0] = t3d_fnf[5];
    t3DFirmwareInfo[0][iClk][1] = t3d_oldfnf[5];
    t3DFirmwareInfo[0][iClk][2] = tsf_fnf[0];
//    t3DFirmwareInfo[0][iClk][3] = result3D->m_t3dtrk0_evtTimeValid_delay;
//    t3DFirmwareInfo[0][iClk][4] = result3D->m_t3dtrk0_evtTime_delay;
    t3DFirmwareInfo[0][iClk][5] = result->m_t2d0_charge;
    t3DFirmwareInfo[0][iClk][6] = result->m_t2d0_rho_s;
    t3DFirmwareInfo[0][iClk][7] = result->m_t2d0_phi;
    t3DFirmwareInfo[0][iClk][8] = result3D->m_t3dtrk0_z0_s;
    t3DFirmwareInfo[0][iClk][9] = result3D->m_t3dtrk0_cot_s;
    t3DFirmwareInfo[0][iClk][10] = result3D->m_t3dtrk0_zchisq;

    t3DFirmwareInfo[1][iClk][0] = t3d_fnf[4];
    t3DFirmwareInfo[1][iClk][1] = t3d_oldfnf[4];
    t3DFirmwareInfo[1][iClk][2] = tsf_fnf[1];
//    t3DFirmwareInfo[1][iClk][3] = result3D->m_t3dtrk1_evtTimeValid_delay;
//    t3DFirmwareInfo[1][iClk][4] = result3D->m_t3dtrk1_evtTime_delay;
    t3DFirmwareInfo[1][iClk][5] = result->m_t2d1_charge;
    t3DFirmwareInfo[1][iClk][6] = result->m_t2d1_rho_s;
    t3DFirmwareInfo[1][iClk][7] = result->m_t2d1_phi;
    t3DFirmwareInfo[1][iClk][8] = result3D->m_t3dtrk1_z0_s;
    t3DFirmwareInfo[1][iClk][9] = result3D->m_t3dtrk1_cot_s;
    t3DFirmwareInfo[1][iClk][10] = result3D->m_t3dtrk1_zchisq;

    t3DFirmwareInfo[2][iClk][0] = t3d_fnf[3];
    t3DFirmwareInfo[2][iClk][1] = t3d_oldfnf[3];
    t3DFirmwareInfo[2][iClk][2] = tsf_fnf[2];
//    t3DFirmwareInfo[2][iClk][3] = result3D->m_t3dtrk2_evtTimeValid_delay;
//    t3DFirmwareInfo[2][iClk][4] = result3D->m_t3dtrk2_evtTime_delay;
    t3DFirmwareInfo[2][iClk][5] = result->m_t2d2_charge;
    t3DFirmwareInfo[2][iClk][6] = result->m_t2d2_rho_s;
    t3DFirmwareInfo[2][iClk][7] = result->m_t2d2_phi;
    t3DFirmwareInfo[2][iClk][8] = result3D->m_t3dtrk2_z0_s;
    t3DFirmwareInfo[2][iClk][9] = result3D->m_t3dtrk2_cot_s;
    t3DFirmwareInfo[2][iClk][10] = result3D->m_t3dtrk2_zchisq;

    t3DFirmwareInfo[3][iClk][0] = t3d_fnf[2];
    t3DFirmwareInfo[3][iClk][1] = t3d_oldfnf[2];
    t3DFirmwareInfo[3][iClk][2] = tsf_fnf[3];
//    t3DFirmwareInfo[3][iClk][3] = result3D->m_t3dtrk3_evtTimeValid_delay;
//    t3DFirmwareInfo[3][iClk][4] = result3D->m_t3dtrk3_evtTime_delay;
    t3DFirmwareInfo[3][iClk][5] = result->m_t2d3_charge;
    t3DFirmwareInfo[3][iClk][6] = result->m_t2d3_rho_s;
    t3DFirmwareInfo[3][iClk][7] = result->m_t2d3_phi;
    t3DFirmwareInfo[3][iClk][8] = result3D->m_t3dtrk3_z0_s;
    t3DFirmwareInfo[3][iClk][9] = result3D->m_t3dtrk3_cot_s;
    t3DFirmwareInfo[3][iClk][10] = result3D->m_t3dtrk3_zchisq;

  }

}

// t3DFirmwareInfo[tIndex][iClk][2DValid, 2DisOld, TSFValid, EventTimeValid, eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::store3DFastSimData(multi_array<double, 3>& t3DFirmwareInfo)
{

  for (int iClk = 0; iClk < m_firmwareResults.getEntries() - 17 - 1; iClk++) {

    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    TRGCDCT3DUnpackerStore* result3D = m_firmwareResults[iClk + 17];
    TRGCDCT3DUnpackerStore* resultDebug = m_firmwareResults[iClk + 4];

    if (bitset<6> (result->m_t2d_fnf)[5] == 0) continue;

    // Store information(charge, radius(cm), phi_c(rad), eventTime, eventTimeValid, [TS ID, TS LR, TS driftTime])
    // [TODO] Is evtTime clock correct?
    int charge = resultDebug->m_t3dtrk0_charge == 2 ? -1 : 1;
    double radius = resultDebug->m_t3dtrk0_rho * 2500 / (pow(2, 11) - 0.5);
    double phi_c = toSigned(resultDebug->m_t3dtrk0_phi0, 13) * M_PI / (pow(2, 12) - 0.5);
    int eventTime = result3D->m_t3dtrk0_evtTime_delay;
    int eventTimeValid = result3D->m_t3dtrk0_evtTimeValid_delay;
    // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
    vector<vector<int> > rawStTSs(4, vector<int> (3));
    rawStTSs[0][0] = resultDebug->m_t3dtrk0ts0_id;
    rawStTSs[0][1] = resultDebug->m_t3dtrk0ts0_lr;
    rawStTSs[0][2] = resultDebug->m_t3dtrk0ts0_rt;
    rawStTSs[1][0] = resultDebug->m_t3dtrk0ts1_id;
    rawStTSs[1][1] = resultDebug->m_t3dtrk0ts1_lr;
    rawStTSs[1][2] = resultDebug->m_t3dtrk0ts1_rt;
    rawStTSs[2][0] = resultDebug->m_t3dtrk0ts2_id;
    rawStTSs[2][1] = resultDebug->m_t3dtrk0ts2_lr;
    rawStTSs[2][2] = resultDebug->m_t3dtrk0ts2_rt;
    rawStTSs[3][0] = resultDebug->m_t3dtrk0ts3_id;
    rawStTSs[3][1] = resultDebug->m_t3dtrk0ts3_lr;
    rawStTSs[3][2] = resultDebug->m_t3dtrk0ts3_rt;

    // Get geometry
    map<string, vector<double> > stGeometry;
    vector<vector<double> > stXts;
    TRGCDCFitter3D::getStereoGeometry(stGeometry);
    TRGCDCFitter3D::getStereoXt(stGeometry["priorityLayer"], stXts);

    double z0 = 0, cot = 0, chi2 = 0;
    Fitter3DUtility::fitter3D(stGeometry, stXts, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, z0, cot, chi2);

    bitset<6> t2d_fnf(int(result3D->m_t3d_2dfnf));
    bitset<4> tsf_fnf(int(result3D->m_t3d_validTS));
    bitset<6> t2d_oldfnf(int(result->m_t3d_2doldtrk));

    //cout<<iClk<<" "<<t2d_fnf<<" "<<t2d_oldfnf<<endl;

    t3DFirmwareInfo[0][iClk][0] = t2d_fnf[5];
    t3DFirmwareInfo[0][iClk][1] = t2d_oldfnf[5];
    t3DFirmwareInfo[0][iClk][2] = tsf_fnf[3];

    t3DFirmwareInfo[0][iClk][3] = result3D->m_t3dtrk0_evtTimeValid_delay;
    t3DFirmwareInfo[0][iClk][4] = result3D->m_t3dtrk0_evtTime_delay;
    t3DFirmwareInfo[0][iClk][5] = charge;
    t3DFirmwareInfo[0][iClk][6] = radius;
    t3DFirmwareInfo[0][iClk][7] = phi_c;
    t3DFirmwareInfo[0][iClk][8] = z0;
    t3DFirmwareInfo[0][iClk][9] = cot;
    t3DFirmwareInfo[0][iClk][10] = chi2;
  }
}

// t3DFirmwareInfo[tIndex][iClk][2DValid, 2DisOld, TSFValid, EventTimeValid, eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::store3DFirmSimData(multi_array<double, 3>& t3DFirmwareInfo)
{
  for (int iClk = 0; iClk < m_firmwareResults.getEntries() - 17 - 1; iClk++) {

    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    TRGCDCT3DUnpackerStore* result3D = m_firmwareResults[iClk + 17];
    TRGCDCT3DUnpackerStore* resultDebug = m_firmwareResults[iClk + 4];

    if (bitset<6> (result->m_t2d_fnf)[5] == 0) continue;

    // Store information(charge, radius(cm), phi_c(rad), eventTime, eventTimeValid, [TS ID, TS LR, TS driftTime])
    // [TODO] Is evtTime clock correct?
    int charge = resultDebug->m_t3dtrk0_charge == 2 ? -1 : 1;
    double radius = resultDebug->m_t3dtrk0_rho * 2500 / (pow(2, 11) - 0.5);
    double phi_c = toSigned(resultDebug->m_t3dtrk0_phi0, 13) * M_PI / (pow(2, 12) - 0.5);
    int eventTime = result3D->m_t3dtrk0_evtTime_delay;
    int eventTimeValid = result3D->m_t3dtrk0_evtTimeValid_delay;
    // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
    vector<vector<int> > rawStTSs(4, vector<int> (3));
    rawStTSs[0][0] = resultDebug->m_t3dtrk0ts0_id;
    rawStTSs[0][1] = resultDebug->m_t3dtrk0ts0_lr;
    rawStTSs[0][2] = resultDebug->m_t3dtrk0ts0_rt;
    rawStTSs[1][0] = resultDebug->m_t3dtrk0ts1_id;
    rawStTSs[1][1] = resultDebug->m_t3dtrk0ts1_lr;
    rawStTSs[1][2] = resultDebug->m_t3dtrk0ts1_rt;
    rawStTSs[2][0] = resultDebug->m_t3dtrk0ts2_id;
    rawStTSs[2][1] = resultDebug->m_t3dtrk0ts2_lr;
    rawStTSs[2][2] = resultDebug->m_t3dtrk0ts2_rt;
    rawStTSs[3][0] = resultDebug->m_t3dtrk0ts3_id;
    rawStTSs[3][1] = resultDebug->m_t3dtrk0ts3_lr;
    rawStTSs[3][2] = resultDebug->m_t3dtrk0ts3_rt;

    // Get geometry
    map<string, vector<double> > stGeometry;
    vector<vector<double> > stXts;
    TRGCDCFitter3D::getStereoGeometry(stGeometry);
    TRGCDCFitter3D::getStereoXt(stGeometry["priorityLayer"], stXts);

    // Prepare complex fit3D
    map<string, vector<double> > mConstV;
    map<string, double> mConstD;
    TRGCDCFitter3D::getConstants(mConstD, mConstV);
    Fitter3DUtility::fitter3DFirm(mConstD, mConstV, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, m_commonData,
                                  m_mSignalStorage, m_mLutStorage);
    double z0 = m_mSignalStorage["z0_r"].getRealInt();
    double cot = m_mSignalStorage["cot_r"].getRealInt();
    double chi2 = m_mSignalStorage["zChi2_r"].getRealInt();

    bitset<6> t2d_fnf(int(result3D->m_t3d_2dfnf));
    bitset<4> tsf_fnf(int(result3D->m_t3d_validTS));
    bitset<6> t2d_oldfnf(int(result->m_t3d_2doldtrk));

    //cout<<iClk<<" "<<t2d_fnf<<" "<<t2d_oldfnf<<endl;

    t3DFirmwareInfo[0][iClk][0] = t2d_fnf[5];
    t3DFirmwareInfo[0][iClk][1] = t2d_oldfnf[5];
    t3DFirmwareInfo[0][iClk][2] = tsf_fnf[3];

    t3DFirmwareInfo[0][iClk][3] = result3D->m_t3dtrk0_evtTimeValid_delay;
    t3DFirmwareInfo[0][iClk][4] = result3D->m_t3dtrk0_evtTime_delay;
    t3DFirmwareInfo[0][iClk][5] = charge;
    t3DFirmwareInfo[0][iClk][6] = radius;
    t3DFirmwareInfo[0][iClk][7] = phi_c;
    t3DFirmwareInfo[0][iClk][8] = z0;
    t3DFirmwareInfo[0][iClk][9] = cot;
    t3DFirmwareInfo[0][iClk][10] = chi2;
  }
}

void TRGCDCT3DConverterModule::debug3DFirmware()
{
  bool doPrint = (m_isVerbose > 1);

  for (int iClk = 0; iClk < m_firmwareResults.getEntries(); iClk++) {
    TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
    //bitset<6> t2d_fnf (int(result->m_t3d_2dfnf));
    //bitset<4> tsf_fnf (int(result->m_t3d_validTS));

    //cout<<"iClk:"<<iClk<<endl;
    //cout<<"  [2D] fnf:"<<result->m_t2d_fnf<<" "<<bitset<6> (result->m_t2d_fnf)<<endl;
    //cout<<"       [0] rho: "<<result->m_t2d0_rho_s<<" "<<toSigned(result->m_t2d0_rho_s, 7)<<" "<<int(0.3*34/30/1.5e-4/abs(toSigned(result->m_t2d0_rho_s, 7))*(pow(2,11)-0.5)/2500)<<" phi0: "<<result->m_t2d0_phi<<endl;
    //cout<<"       [1] rho: "<<result->m_t2d1_rho_s<<" "<<toSigned(result->m_t2d1_rho_s, 7)<<" "<<int(0.3*34/30/1.5e-4/abs(toSigned(result->m_t2d1_rho_s, 7))*(pow(2,11)-0.5)/2500)<<" phi0: "<<result->m_t2d1_phi<<endl;
    //cout<<"       [2] rho: "<<result->m_t2d2_rho_s<<" "<<toSigned(result->m_t2d2_rho_s, 7)<<" "<<int(0.3*34/30/1.5e-4/abs(toSigned(result->m_t2d2_rho_s, 7))*(pow(2,11)-0.5)/2500)<<" phi0: "<<result->m_t2d2_phi<<endl;
    //cout<<"       [3] rho: "<<result->m_t2d3_rho_s<<" "<<toSigned(result->m_t2d3_rho_s, 7)<<" "<<int(0.3*34/30/1.5e-4/abs(toSigned(result->m_t2d3_rho_s, 7))*(pow(2,11)-0.5)/2500)<<" phi0: "<<result->m_t2d3_phi<<endl;
    //cout<<"  [3D] fnf:"<<result->m_t3d_2dfnf<<" "<<t2d_fnf<<" "<<t2d_fnf[5]<<" tsValid: "<<result->m_t3d_validTS<<" "<<tsf_fnf<<endl;;
    //cout<<"       [0] z0: "<<result->m_t3dtrk0_z0_s<<" cot: "<<result->m_t3dtrk0_cot_s<<" chi2: "<<result->m_t3dtrk0_zchisq<<endl;
    //cout<<"       [1] z0: "<<result->m_t3dtrk1_z0_s<<" cot: "<<result->m_t3dtrk1_cot_s<<" chi2: "<<result->m_t3dtrk1_zchisq<<endl;
    //cout<<"       [2] z0: "<<result->m_t3dtrk2_z0_s<<" cot: "<<result->m_t3dtrk2_cot_s<<" chi2: "<<result->m_t3dtrk2_zchisq<<endl;
    //cout<<"       [3] z0: "<<result->m_t3dtrk3_z0_s<<" cot: "<<result->m_t3dtrk3_cot_s<<" chi2: "<<result->m_t3dtrk3_zchisq<<endl;
    //cout<<"       [debug] MSB"<<endl;
    //cout<<"         evtTime: "<<result->m_t3dtrk0_evtTime_delay<<" evtTimeValid: "<<result->m_t3dtrk0_evtTimeValid_delay<<endl;
    //cout<<"         charge: "<<result->m_t3dtrk0_charge<<" rho: "<<result->m_t3dtrk0_rho<<" phi0: "<<result->m_t3dtrk0_phi0<<endl;;
    //cout<<"         ts0 id: "<<result->m_t3dtrk0ts0_id<<" lr: "<<result->m_t3dtrk0ts0_lr<<" rt: "<<result->m_t3dtrk0ts0_rt<<endl;
    //cout<<"         ts1 id: "<<result->m_t3dtrk0ts1_id<<" lr: "<<result->m_t3dtrk0ts1_lr<<" rt: "<<result->m_t3dtrk0ts1_rt<<endl;
    //cout<<"         ts2 id: "<<result->m_t3dtrk0ts2_id<<" lr: "<<result->m_t3dtrk0ts2_lr<<" rt: "<<result->m_t3dtrk0ts2_rt<<endl;
    //cout<<"         ts3 id: "<<result->m_t3dtrk0ts3_id<<" lr: "<<result->m_t3dtrk0ts3_lr<<" rt: "<<result->m_t3dtrk0ts3_rt<<endl;
    //cout<<"       [debug] LSB"<<endl;
    //cout<<"         evtTime: "<<result->m_t3dtrk1_evtTime_delay<<" evtTimeValid: "<<result->m_t3dtrk1_evtTimeValid_delay<<endl;
    //cout<<"         charge: "<<result->m_t3dtrk1_charge<<" rho: "<<result->m_t3dtrk1_rho<<" phi0: "<<result->m_t3dtrk1_phi0<<endl;;
    //cout<<"         ts0 id: "<<result->m_t3dtrk1ts0_id<<" lr: "<<result->m_t3dtrk1ts0_lr<<" rt: "<<result->m_t3dtrk1ts0_rt<<endl;
    //cout<<"         ts1 id: "<<result->m_t3dtrk1ts1_id<<" lr: "<<result->m_t3dtrk1ts1_lr<<" rt: "<<result->m_t3dtrk1ts1_rt<<endl;
    //cout<<"         ts2 id: "<<result->m_t3dtrk1ts2_id<<" lr: "<<result->m_t3dtrk1ts2_lr<<" rt: "<<result->m_t3dtrk1ts2_rt<<endl;
    //cout<<"         ts3 id: "<<result->m_t3dtrk1ts3_id<<" lr: "<<result->m_t3dtrk1ts3_lr<<" rt: "<<result->m_t3dtrk1ts3_rt<<endl;

    //// Compare 2D0 with debug after 4 clocks
    //if (result->m_t2d_fnf && iClk < 48-4-1 && bitset<6> (result->m_t2d_fnf)[5])
    //{
    //  TRGCDCT3DUnpackerStore * resultDebug = m_firmwareResults[iClk+4];
    //  if(t2DRhoTot3DRho(result->m_t2d0_rho_s) == resultDebug->m_t3dtrk0_rho)
    //  {
    //    cout<<"Matched 2D0 and debug0"<<endl;
    //  } else {
    //    cout<<"iClk:"<<iClk<<endl;
    //    cout<<"  [2D] fnf:"<<result->m_t2d_fnf<<" "<<bitset<6> (result->m_t2d_fnf)<<endl;
    //    cout<<"       [0] rho: "<<result->m_t2d0_rho_s<<" "<<toSigned(result->m_t2d0_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d0_rho_s)<<" phi0: "<<result->m_t2d0_phi<<endl;
    //    cout<<"       [1] rho: "<<result->m_t2d1_rho_s<<" "<<toSigned(result->m_t2d1_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d1_rho_s)<<" phi0: "<<result->m_t2d1_phi<<endl;
    //    cout<<"       [2] rho: "<<result->m_t2d2_rho_s<<" "<<toSigned(result->m_t2d2_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d2_rho_s)<<" phi0: "<<result->m_t2d2_phi<<endl;
    //    cout<<"       [3] rho: "<<result->m_t2d3_rho_s<<" "<<toSigned(result->m_t2d3_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d3_rho_s)<<" phi0: "<<result->m_t2d3_phi<<endl;
    //    cout<<"  [3DDebug] MSB"<<endl;
    //    cout<<"         charge: "<<resultDebug->m_t3dtrk0_charge<<" rho: "<<resultDebug->m_t3dtrk0_rho<<" phi0: "<<resultDebug->m_t3dtrk0_phi0<<endl;;
    //    cout<<"         ts0 id: "<<resultDebug->m_t3dtrk0ts0_id<<" lr: "<<resultDebug->m_t3dtrk0ts0_lr<<" rt: "<<resultDebug->m_t3dtrk0ts0_rt<<endl;
    //    cout<<"         ts1 id: "<<resultDebug->m_t3dtrk0ts1_id<<" lr: "<<resultDebug->m_t3dtrk0ts1_lr<<" rt: "<<resultDebug->m_t3dtrk0ts1_rt<<endl;
    //    cout<<"         ts2 id: "<<resultDebug->m_t3dtrk0ts2_id<<" lr: "<<resultDebug->m_t3dtrk0ts2_lr<<" rt: "<<resultDebug->m_t3dtrk0ts2_rt<<endl;
    //    cout<<"         ts3 id: "<<resultDebug->m_t3dtrk0ts3_id<<" lr: "<<resultDebug->m_t3dtrk0ts3_lr<<" rt: "<<resultDebug->m_t3dtrk0ts3_rt<<endl;
    //  }
    //}

    //// Compare 2D3 with debug after 4 clocks
    //if (iClk < 48-4-1 && bitset<6> (result->m_t2d_fnf)[2])
    //{
    //  TRGCDCT3DUnpackerStore * resultDebug = m_firmwareResults[iClk+4];
    //  if(t2DRhoTot3DRho(result->m_t2d3_rho_s) == resultDebug->m_t3dtrk1_rho)
    //  {
    //    cout<<"Matched 2D3 and debug1"<<endl;
    //  } else {
    //    cout<<"iClk:"<<iClk<<endl;
    //    cout<<"  [2D] fnf:"<<result->m_t2d_fnf<<" "<<bitset<6> (result->m_t2d_fnf)<<endl;
    //    cout<<"       [0] rho: "<<result->m_t2d0_rho_s<<" "<<toSigned(result->m_t2d0_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d0_rho_s)<<" phi0: "<<result->m_t2d0_phi<<endl;
    //    cout<<"       [1] rho: "<<result->m_t2d1_rho_s<<" "<<toSigned(result->m_t2d1_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d1_rho_s)<<" phi0: "<<result->m_t2d1_phi<<endl;
    //    cout<<"       [2] rho: "<<result->m_t2d2_rho_s<<" "<<toSigned(result->m_t2d2_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d2_rho_s)<<" phi0: "<<result->m_t2d2_phi<<endl;
    //    cout<<"       [3] rho: "<<result->m_t2d3_rho_s<<" "<<toSigned(result->m_t2d3_rho_s, 7)<<" "<<t2DRhoTot3DRho(result->m_t2d3_rho_s)<<" phi0: "<<result->m_t2d3_phi<<endl;
    //    cout<<"  [3DDebug] LSB"<<endl;
    //    cout<<"         evtTime: "<<resultDebug->m_t3dtrk1_evtTime_delay<<" evtTimeValid: "<<resultDebug->m_t3dtrk1_evtTimeValid_delay<<endl;
    //    cout<<"         charge: "<<resultDebug->m_t3dtrk1_charge<<" rho: "<<resultDebug->m_t3dtrk1_rho<<" phi0: "<<resultDebug->m_t3dtrk1_phi0<<endl;;
    //    cout<<"         ts0 id: "<<resultDebug->m_t3dtrk1ts0_id<<" lr: "<<resultDebug->m_t3dtrk1ts0_lr<<" rt: "<<resultDebug->m_t3dtrk1ts0_rt<<endl;
    //    cout<<"         ts1 id: "<<resultDebug->m_t3dtrk1ts1_id<<" lr: "<<resultDebug->m_t3dtrk1ts1_lr<<" rt: "<<resultDebug->m_t3dtrk1ts1_rt<<endl;
    //    cout<<"         ts2 id: "<<resultDebug->m_t3dtrk1ts2_id<<" lr: "<<resultDebug->m_t3dtrk1ts2_lr<<" rt: "<<resultDebug->m_t3dtrk1ts2_rt<<endl;
    //    cout<<"         ts3 id: "<<resultDebug->m_t3dtrk1ts3_id<<" lr: "<<resultDebug->m_t3dtrk1ts3_lr<<" rt: "<<resultDebug->m_t3dtrk1ts3_rt<<endl;
    //  }
    //}

    //// Compare debug with 3DTSF0 after 7 clks
    //int countTrk0 = int(result->m_t3dtrk0ts0_lr!=0)+int(result->m_t3dtrk0ts1_lr!=0)+int(result->m_t3dtrk0ts2_lr!=0)+int(result->m_t3dtrk0ts3_lr!=0);
    //if ( iClk < 48 - 7 - 1)
    //{
    //  TRGCDCT3DUnpackerStore * resultDebug = m_firmwareResults[iClk];
    //  TRGCDCT3DUnpackerStore * result = m_firmwareResults[iClk+7];

    //  if (int(countTrk0 > 1) == bitset<4> (int(result->m_t3d_validTS))[3])
    //  {
    //    cout<<"debug TSF matches with TSFValid0"<<endl;
    //  } else {
    //    cout<<"iClk:"<<iClk+7<<endl;
    //    cout<<"  [3D] fnf:"<<result->m_t3d_2dfnf<<" "<<bitset<6> (int(result->m_t3d_2dfnf))<<" tsValid: "<<result->m_t3d_validTS<<" "<<bitset<4> (int(result->m_t3d_validTS))<<endl;;
    //    cout<<"  [3DDebug] MSB"<<endl;
    //    cout<<"         charge: "<<resultDebug->m_t3dtrk0_charge<<" rho: "<<resultDebug->m_t3dtrk0_rho<<" phi0: "<<resultDebug->m_t3dtrk0_phi0<<endl;;
    //    cout<<"         ts0 id: "<<resultDebug->m_t3dtrk0ts0_id<<" lr: "<<resultDebug->m_t3dtrk0ts0_lr<<" rt: "<<resultDebug->m_t3dtrk0ts0_rt<<endl;
    //    cout<<"         ts1 id: "<<resultDebug->m_t3dtrk0ts1_id<<" lr: "<<resultDebug->m_t3dtrk0ts1_lr<<" rt: "<<resultDebug->m_t3dtrk0ts1_rt<<endl;
    //    cout<<"         ts2 id: "<<resultDebug->m_t3dtrk0ts2_id<<" lr: "<<resultDebug->m_t3dtrk0ts2_lr<<" rt: "<<resultDebug->m_t3dtrk0ts2_rt<<endl;
    //    cout<<"         ts3 id: "<<resultDebug->m_t3dtrk0ts3_id<<" lr: "<<resultDebug->m_t3dtrk0ts3_lr<<" rt: "<<resultDebug->m_t3dtrk0ts3_rt<<endl;
    //    cout<<"         count: "<<countTrk0<<endl;
    //  }
    //}

    //// Compare debug with 3DTSF3 after 7 clks
    //int countTrk3 = int(result->m_t3dtrk1ts0_lr!=0)+int(result->m_t3dtrk1ts1_lr!=0)+int(result->m_t3dtrk1ts2_lr!=0)+int(result->m_t3dtrk1ts3_lr!=0);
    //if ( iClk < 48 - 7 - 1)
    //{
    //  TRGCDCT3DUnpackerStore * resultDebug = m_firmwareResults[iClk];
    //  TRGCDCT3DUnpackerStore * result = m_firmwareResults[iClk+7];

    //  if (int(countTrk3 > 1) == bitset<4> (int(result->m_t3d_validTS))[2])
    //  {
    //    cout<<"debug TSF matches with TSFValid3"<<endl;
    //  } else {
    //    cout<<"iClk:"<<iClk+7<<endl;
    //    cout<<"  [3D] fnf:"<<result->m_t3d_2dfnf<<" "<<bitset<6> (int(result->m_t3d_2dfnf))<<" tsValid: "<<result->m_t3d_validTS<<" "<<bitset<4> (int(result->m_t3d_validTS))<<endl;;
    //    cout<<"  [3DDebug] LSB"<<endl;
    //    cout<<"         charge: "<<resultDebug->m_t3dtrk1_charge<<" rho: "<<resultDebug->m_t3dtrk1_rho<<" phi0: "<<resultDebug->m_t3dtrk1_phi0<<endl;;
    //    cout<<"         ts0 id: "<<resultDebug->m_t3dtrk1ts0_id<<" lr: "<<resultDebug->m_t3dtrk1ts0_lr<<" rt: "<<resultDebug->m_t3dtrk1ts0_rt<<endl;
    //    cout<<"         ts1 id: "<<resultDebug->m_t3dtrk1ts1_id<<" lr: "<<resultDebug->m_t3dtrk1ts1_lr<<" rt: "<<resultDebug->m_t3dtrk1ts1_rt<<endl;
    //    cout<<"         ts2 id: "<<resultDebug->m_t3dtrk1ts2_id<<" lr: "<<resultDebug->m_t3dtrk1ts2_lr<<" rt: "<<resultDebug->m_t3dtrk1ts2_rt<<endl;
    //    cout<<"         ts3 id: "<<resultDebug->m_t3dtrk1ts3_id<<" lr: "<<resultDebug->m_t3dtrk1ts3_lr<<" rt: "<<resultDebug->m_t3dtrk1ts3_rt<<endl;
    //    cout<<"         count: "<<countTrk3<<endl;
    //  }
    //}

    // Compare 2D with debug and 3D after 4 clks and 11 clks
    if (result->m_t2d_fnf && iClk < 48 - 17 - 1 && bitset<6> (result->m_t2d_fnf)[5]) {
      TRGCDCT3DUnpackerStore* resultDebug = m_firmwareResults[iClk + 4];
      TRGCDCT3DUnpackerStore* result3D = m_firmwareResults[iClk + 17];

      // Store information(charge, radius(cm), phi_c(rad), eventTime, eventTimeValid, [TS ID, TS LR, TS driftTime])
      // [TODO] Is evtTime clock correct?
      int charge = resultDebug->m_t3dtrk0_charge;
      double radius = resultDebug->m_t3dtrk0_rho * 2500 / (pow(2, 11) - 0.5);
      double phi_c = toSigned(resultDebug->m_t3dtrk0_phi0, 13) * M_PI / (pow(2, 12) - 0.5);
      int eventTime = result3D->m_t3dtrk0_evtTime_delay;
      int eventTimeValid = result3D->m_t3dtrk0_evtTimeValid_delay;
      // rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
      vector<vector<int> > rawStTSs(4, vector<int> (3));
      rawStTSs[0][0] = resultDebug->m_t3dtrk0ts0_id;
      rawStTSs[0][1] = resultDebug->m_t3dtrk0ts0_lr;
      rawStTSs[0][2] = resultDebug->m_t3dtrk0ts0_rt;
      rawStTSs[1][0] = resultDebug->m_t3dtrk0ts1_id;
      rawStTSs[1][1] = resultDebug->m_t3dtrk0ts1_lr;
      rawStTSs[1][2] = resultDebug->m_t3dtrk0ts1_rt;
      rawStTSs[2][0] = resultDebug->m_t3dtrk0ts2_id;
      rawStTSs[2][1] = resultDebug->m_t3dtrk0ts2_lr;
      rawStTSs[2][2] = resultDebug->m_t3dtrk0ts2_rt;
      rawStTSs[3][0] = resultDebug->m_t3dtrk0ts3_id;
      rawStTSs[3][1] = resultDebug->m_t3dtrk0ts3_lr;
      rawStTSs[3][2] = resultDebug->m_t3dtrk0ts3_rt;

      // Get geometry
      map<string, vector<double> > stGeometry;
      vector<vector<double> > stXts;
      TRGCDCFitter3D::getStereoGeometry(stGeometry);
      TRGCDCFitter3D::getStereoXt(stGeometry["priorityLayer"], stXts);
      //Fitter3DUtility::loadStereoXt("TODOdata/stereoXt",4,stXts);

      //// Calcualte phi3D
      //vector<double> stTSs(4);
      //Fitter3DUtility::calPhiFast(stGeometry, stXts, eventTimeValid, eventTime, rawStTSs, stTSs);
      //cout<<"    [calPhi] "<<stTSs[0]<<" "<<stTSs[1]<<" "<<stTSs[2]<<" "<<stTSs[3]<<endl;
      //// Get zError for 3D
      //vector<double> invZError2;
      //Fitter3DUtility::setErrorFast(rawStTSs, eventTimeValid, invZError2);
      //cout<<"    [invZError2] "<<invZError2[0]<<" "<<invZError2[1]<<" "<<invZError2[2]<<" "<<invZError2[3]<<endl;
      //// Calculate zz and arcS
      //// rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
      //vector<double> zz(4, 0);
      //vector<double> arcS(4, 0);
      //for (unsigned iSt = 0; iSt < 4; iSt++) {
      //  if (rawStTSs[iSt][1] != 0) {
      //    zz[iSt] = Fitter3DUtility::calZ(charge, stGeometry["angleSt"][iSt], stGeometry["zToStraw"][iSt],
      //                                    stGeometry["cdcRadius"][iSt], stTSs[iSt], radius, phi_c);
      //    arcS[iSt] = Fitter3DUtility::calS(radius, stGeometry["cdcRadius"][iSt]);
      //  }
      //}
      //cout<<"    [zz] "<<zz[0]<<" "<<zz[1]<<" "<<zz[2]<<" "<<zz[3]<<endl;
      //cout<<"    [arcS] "<<arcS[0]<<" "<<arcS[1]<<" "<<arcS[2]<<" "<<arcS[3]<<endl;
      //// Fit 3D
      //double z0 = 0;
      //double cot = 0;
      //double chi2 = 0;
      //Fitter3DUtility::rSFit(&invZError2[0], &arcS[0], &zz[0], z0, cot, chi2);
      double z0 = 0;
      double cot = 0;
      double chi2 = 0;
      Fitter3DUtility::fitter3D(stGeometry, stXts, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, z0, cot, chi2);

      // Prepare complex fit3D
      map<string, vector<double> > mConstV;
      map<string, double> mConstD;
      TRGCDCFitter3D::getConstants(mConstD, mConstV);
      //Belle2::TRGCDCJSignalData commonData;
      //map<string, TRGCDCJSignal> mSignalStorage;
      //map<string, TRGCDCJLUT*> mLutStorage;
      Fitter3DUtility::fitter3DFirm(mConstD, mConstV, eventTimeValid, eventTime, rawStTSs, charge, radius, phi_c, m_commonData,
                                    m_mSignalStorage, m_mLutStorage);

      //// Change to Signals.
      //// rawStTSs[iSt] = [TS ID, TS LR, TS driftTime]
      //vector<tuple<string, double, int, double, double, int> > t_values = {
      //  make_tuple("phi0", phi_c, mConstD["phiBitSize"], mConstD["phiMin"], mConstD["phiMax"], 0),
      //  make_tuple("rho", radius, mConstD["rhoBitSize"], mConstD["rhoMin"], mConstD["rhoMax"], 0),
      //  //make_tuple("charge",(int) (m_mDouble["charge"]==1 ? 1 : 0), 1, 0, 1.5, 0),
      //  make_tuple("charge", (int)(charge == 1 ? 1 : 0), 1, 0, 1.5, 0),
      //  make_tuple("lr_0", rawStTSs[0][1], 2, 0, 3.5, 0),
      //  make_tuple("lr_1", rawStTSs[1][1], 2, 0, 3.5, 0),
      //  make_tuple("lr_2", rawStTSs[2][1], 2, 0, 3.5, 0),
      //  make_tuple("lr_3", rawStTSs[3][1], 2, 0, 3.5, 0),
      //  make_tuple("tsId_0", rawStTSs[0][0], ceil(log(mConstV["nTSs"][1]) / log(2)), 0, pow(2, ceil(log(mConstV["nTSs"][1]) / log(2))) - 0.5, 0),
      //  make_tuple("tsId_1", rawStTSs[1][0], ceil(log(mConstV["nTSs"][3]) / log(2)), 0, pow(2, ceil(log(mConstV["nTSs"][3]) / log(2))) - 0.5, 0),
      //  make_tuple("tsId_2", rawStTSs[2][0], ceil(log(mConstV["nTSs"][5]) / log(2)), 0, pow(2, ceil(log(mConstV["nTSs"][5]) / log(2))) - 0.5, 0),
      //  make_tuple("tsId_3", rawStTSs[3][0], ceil(log(mConstV["nTSs"][7]) / log(2)), 0, pow(2, ceil(log(mConstV["nTSs"][7]) / log(2))) - 0.5, 0),
      //  make_tuple("tdc_0", rawStTSs[0][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
      //  make_tuple("tdc_1", rawStTSs[1][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
      //  make_tuple("tdc_2", rawStTSs[2][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
      //  make_tuple("tdc_3", rawStTSs[3][2], mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
      //  make_tuple("eventTime", eventTime, mConstD["tdcBitSize"], 0, pow(2, mConstD["tdcBitSize"]) - 0.5, 0),
      //  make_tuple("eventTimeValid", eventTimeValid, 1, 0, 1.5, 0),
      //};
      //TRGCDCJSignal::valuesToMapSignals(t_values, m_commonData, m_mSignalStorage);
      //// Calculate complex fit3D
      //Fitter3DUtility::setError(mConstD, mConstV, m_mSignalStorage);
      //Fitter3DUtility::calPhi(mConstD, mConstV, m_mSignalStorage, m_mLutStorage);
      //Fitter3DUtility::constrainRPerStSl(mConstV, m_mSignalStorage);
      //Fitter3DUtility::calZ(mConstD, mConstV, m_mSignalStorage, m_mLutStorage);
      //Fitter3DUtility::calS(mConstD, mConstV, m_mSignalStorage, m_mLutStorage);
      //Fitter3DUtility::rSFit(mConstD, mConstV, m_mSignalStorage, m_mLutStorage);
      //// Post handling of signals.
      //// Name all signals.
      //if ((*m_mSignalStorage.begin()).second.getName() == "") {
      //  for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it) {
      //    (*it).second.setName((*it).first);
      //  }
      //}

      if (toSigned(result3D->m_t3dtrk0_z0_s, 11) != m_mSignalStorage["z0_r"].getInt()
          || toSigned(result3D->m_t3dtrk0_cot_s, 11) != m_mSignalStorage["cot_r"].getInt()) {
        bitset<6> t2d_oldfnf(int(result->m_t3d_2doldtrk));
        cout << "iClk:" << iClk << endl;
        cout << "  [2D] fnf:" << result->m_t2d_fnf << " " << bitset<6> (result->m_t2d_fnf) << " oldfnf: " << t2d_oldfnf << endl;
//        cout << "    [0] rho: " << result->m_t2d0_rho_s << " " << t2DRhoTot3DRho(result->m_t2d0_rho_s) << " phi0: " << result->m_t2d0_phi <<
//             " " << t2DPhiTot3DPhi(result->m_t2d0_phi, result->m_t2d0_rho_s) << endl;
        cout << "  [debug] MSB" << endl;
        cout << "    charge: " << resultDebug->m_t3dtrk0_charge << " rho: " << resultDebug->m_t3dtrk0_rho << " phi0: " << toSigned(
               resultDebug->m_t3dtrk0_phi0, 13) << endl;;
        cout << "    ts0 id: " << resultDebug->m_t3dtrk0ts0_id << " lr: " << resultDebug->m_t3dtrk0ts0_lr << " rt: " <<
             resultDebug->m_t3dtrk0ts0_rt << endl;
        cout << "    ts1 id: " << resultDebug->m_t3dtrk0ts1_id << " lr: " << resultDebug->m_t3dtrk0ts1_lr << " rt: " <<
             resultDebug->m_t3dtrk0ts1_rt << endl;
        cout << "    ts2 id: " << resultDebug->m_t3dtrk0ts2_id << " lr: " << resultDebug->m_t3dtrk0ts2_lr << " rt: " <<
             resultDebug->m_t3dtrk0ts2_rt << endl;
        cout << "    ts3 id: " << resultDebug->m_t3dtrk0ts3_id << " lr: " << resultDebug->m_t3dtrk0ts3_lr << " rt: " <<
             resultDebug->m_t3dtrk0ts3_rt << endl;
        cout << "  [3D] fnf:" << result3D->m_t3d_2dfnf << " " << bitset<6> (result3D->m_t3d_2dfnf) << " tsValid: " <<
             result3D->m_t3d_validTS << " " << bitset<4> (result3D->m_t3d_validTS) << endl;;
        cout << "    [0] z0: " << result3D->m_t3dtrk0_z0_s << " " << toSigned(result3D->m_t3dtrk0_z0_s,
             11) << " " << toSigned(result3D->m_t3dtrk0_z0_s,
                                    11) * 0.0382 << " cot: " << result3D->m_t3dtrk0_cot_s << " " << toSigned(result3D->m_t3dtrk0_cot_s,
                                        11) << " " << toSigned(result3D->m_t3dtrk0_cot_s, 11) * 0.00195 << " chi2: " << result3D->m_t3dtrk0_zchisq << endl;
        cout << "    [1] z0: " << result3D->m_t3dtrk1_z0_s << " " << toSigned(result3D->m_t3dtrk1_z0_s,
             11) << " " << toSigned(result3D->m_t3dtrk1_z0_s,
                                    11) * 0.0382 << " cot: " << result3D->m_t3dtrk1_cot_s << " " << toSigned(result3D->m_t3dtrk1_cot_s,
                                        11) << " " << toSigned(result3D->m_t3dtrk1_cot_s, 11) * 0.00195 << " chi2: " << result3D->m_t3dtrk1_zchisq << endl;
        cout << "    [2] z0: " << result3D->m_t3dtrk2_z0_s << " " << toSigned(result3D->m_t3dtrk2_z0_s,
             11) << " " << toSigned(result3D->m_t3dtrk2_z0_s,
                                    11) * 0.0382 << " cot: " << result3D->m_t3dtrk2_cot_s << " " << toSigned(result3D->m_t3dtrk2_cot_s,
                                        11) << " " << toSigned(result3D->m_t3dtrk2_cot_s, 11) * 0.00195 << " chi2: " << result3D->m_t3dtrk2_zchisq << endl;
        cout << "    [3] z0: " << result3D->m_t3dtrk3_z0_s << " " << toSigned(result3D->m_t3dtrk3_z0_s,
             11) << " " << toSigned(result3D->m_t3dtrk3_z0_s,
                                    11) * 0.0382 << " cot: " << result3D->m_t3dtrk3_cot_s << " " << toSigned(result3D->m_t3dtrk3_cot_s,
                                        11) << " " << toSigned(result3D->m_t3dtrk3_cot_s, 11) * 0.00195 << " chi2: " << result3D->m_t3dtrk3_zchisq << endl;
        cout << "    evtTime: " << result3D->m_t3dtrk0_evtTime_delay << " evtTimeValid: " << result3D->m_t3dtrk0_evtTimeValid_delay << endl;

        cout << "  [Store information]" << endl;
        cout << "    charge: " << charge << " radius(cm): " << radius << " phi_c(rad): " << phi_c << endl;
        cout << "    eventTime: " << eventTime << " valid: " << eventTimeValid << endl;
        cout << "    rawStTSs[0] id:" << rawStTSs[0][0] << " lr: " << rawStTSs[0][1] << " rt: " << rawStTSs[0][2] << endl;
        cout << "    rawStTSs[1] id:" << rawStTSs[1][0] << " lr: " << rawStTSs[1][1] << " rt: " << rawStTSs[1][2] << endl;
        cout << "    rawStTSs[2] id:" << rawStTSs[2][0] << " lr: " << rawStTSs[2][1] << " rt: " << rawStTSs[2][2] << endl;
        cout << "    rawStTSs[3] id:" << rawStTSs[3][0] << " lr: " << rawStTSs[3][1] << " rt: " << rawStTSs[3][2] << endl;

        cout << "  [Calculate fast]" << endl;
        cout << "    [z0] " << z0 << " [cot] " << cot << " [chi2] " << chi2 << endl;

        cout << "  [Calculate firm]" << endl;
        cout << "    z0: " << m_mSignalStorage["z0_r"].getInt() << " " << m_mSignalStorage["z0_r"].getRealInt() << " " <<
             m_mSignalStorage["z0_r"].getToReal() << " cot: " << m_mSignalStorage["cot_r"].getInt() << " " <<
             m_mSignalStorage["cot_r"].getRealInt() << " " << m_mSignalStorage["cot_r"].getToReal() << " zchi: " <<
             m_mSignalStorage["zChi2_r"].getInt() << " " << m_mSignalStorage["zChi2_r"].getRealInt() << " " <<
             m_mSignalStorage["zChi2_r"].getToReal() << endl;
        //doPrint = 1;
      }
      //else
      //{
      //  cout<<"same"<<endl;
      //}

    }

  }

  if (doPrint) {
    for (int iClk = 0; iClk < m_firmwareResults.getEntries(); iClk++) {
      TRGCDCT3DUnpackerStore* result = m_firmwareResults[iClk];
      bitset<6> t2d_fnf(int(result->m_t3d_2dfnf));
      bitset<4> tsf_fnf(int(result->m_t3d_validTS));
      bitset<6> t2d_oldfnf(int(result->m_t3d_2doldtrk));
      cout << "iClk:" << iClk << endl;
      cout << "  [2D] fnf:" << result->m_t2d_fnf << " " << bitset<6> (result->m_t2d_fnf) << " oldfnf: " << t2d_oldfnf << endl;
      cout << "       [0] rho: " << result->m_t2d0_rho_s << " " << toSigned(result->m_t2d0_rho_s,
           7) << " " << int(0.3 * 34 / 30 / 1.5e-4 / abs(toSigned(result->m_t2d0_rho_s, 7)) * (pow(2,
                            11) - 0.5) / 2500) << " phi0: " << result->m_t2d0_phi << endl;
      cout << "       [1] rho: " << result->m_t2d1_rho_s << " " << toSigned(result->m_t2d1_rho_s,
           7) << " " << int(0.3 * 34 / 30 / 1.5e-4 / abs(toSigned(result->m_t2d1_rho_s, 7)) * (pow(2,
                            11) - 0.5) / 2500) << " phi0: " << result->m_t2d1_phi << endl;
      cout << "       [2] rho: " << result->m_t2d2_rho_s << " " << toSigned(result->m_t2d2_rho_s,
           7) << " " << int(0.3 * 34 / 30 / 1.5e-4 / abs(toSigned(result->m_t2d2_rho_s, 7)) * (pow(2,
                            11) - 0.5) / 2500) << " phi0: " << result->m_t2d2_phi << endl;
      cout << "       [3] rho: " << result->m_t2d3_rho_s << " " << toSigned(result->m_t2d3_rho_s,
           7) << " " << int(0.3 * 34 / 30 / 1.5e-4 / abs(toSigned(result->m_t2d3_rho_s, 7)) * (pow(2,
                            11) - 0.5) / 2500) << " phi0: " << result->m_t2d3_phi << endl;
      cout << "  [3D] fnf:" << result->m_t3d_2dfnf << " " << t2d_fnf << " " << t2d_fnf[5] << " tsValid: " << result->m_t3d_validTS << " "
           << tsf_fnf << endl;;
      cout << "       [0] z0: " << result->m_t3dtrk0_z0_s << " cot: " << result->m_t3dtrk0_cot_s << " chi2: " << result->m_t3dtrk0_zchisq
           << endl;
      cout << "       [1] z0: " << result->m_t3dtrk1_z0_s << " cot: " << result->m_t3dtrk1_cot_s << " chi2: " << result->m_t3dtrk1_zchisq
           << endl;
      cout << "       [2] z0: " << result->m_t3dtrk2_z0_s << " cot: " << result->m_t3dtrk2_cot_s << " chi2: " << result->m_t3dtrk2_zchisq
           << endl;
      cout << "       [3] z0: " << result->m_t3dtrk3_z0_s << " cot: " << result->m_t3dtrk3_cot_s << " chi2: " << result->m_t3dtrk3_zchisq
           << endl;
      cout << "       [debug] MSB" << endl;
      cout << "         evtTime: " << result->m_t3dtrk0_evtTime_delay << " evtTimeValid: " << result->m_t3dtrk0_evtTimeValid_delay <<
           endl;
      cout << "         charge: " << result->m_t3dtrk0_charge << " rho: " << result->m_t3dtrk0_rho << " phi0: " << result->m_t3dtrk0_phi0
           << endl;;
      cout << "         ts0 id: " << result->m_t3dtrk0ts0_id << " lr: " << result->m_t3dtrk0ts0_lr << " rt: " << result->m_t3dtrk0ts0_rt
           << endl;
      cout << "         ts1 id: " << result->m_t3dtrk0ts1_id << " lr: " << result->m_t3dtrk0ts1_lr << " rt: " << result->m_t3dtrk0ts1_rt
           << endl;
      cout << "         ts2 id: " << result->m_t3dtrk0ts2_id << " lr: " << result->m_t3dtrk0ts2_lr << " rt: " << result->m_t3dtrk0ts2_rt
           << endl;
      cout << "         ts3 id: " << result->m_t3dtrk0ts3_id << " lr: " << result->m_t3dtrk0ts3_lr << " rt: " << result->m_t3dtrk0ts3_rt
           << endl;
      cout << "       [debug] LSB" << endl;
      cout << "         evtTime: " << result->m_t3dtrk1_evtTime_delay << " evtTimeValid: " << result->m_t3dtrk1_evtTimeValid_delay <<
           endl;
      cout << "         charge: " << result->m_t3dtrk1_charge << " rho: " << result->m_t3dtrk1_rho << " phi0: " << result->m_t3dtrk1_phi0
           << endl;;
      cout << "         ts0 id: " << result->m_t3dtrk1ts0_id << " lr: " << result->m_t3dtrk1ts0_lr << " rt: " << result->m_t3dtrk1ts0_rt
           << endl;
      cout << "         ts1 id: " << result->m_t3dtrk1ts1_id << " lr: " << result->m_t3dtrk1ts1_lr << " rt: " << result->m_t3dtrk1ts1_rt
           << endl;
      cout << "         ts2 id: " << result->m_t3dtrk1ts2_id << " lr: " << result->m_t3dtrk1ts2_lr << " rt: " << result->m_t3dtrk1ts2_rt
           << endl;
      cout << "         ts3 id: " << result->m_t3dtrk1ts3_id << " lr: " << result->m_t3dtrk1ts3_lr << " rt: " << result->m_t3dtrk1ts3_rt
           << endl;
    }
  }
}


// t3DFirmwareInfo[tIndex][iClk][2DValid, 2DisOld, TSFValid, EventTimeValid, eventTime, charge, rho, phi0, z0, cot, zchi]
// t3DInfo[eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::filter3DData(multi_array<double, 3>& t3DFirmwareInfo,
                                            multi_array<double, 2>& t3DInfo)
{
  unsigned iTrack_filter = 0;
  for (unsigned iTrack = 0; iTrack < t3DFirmwareInfo.shape()[0]; iTrack++) {
    for (unsigned iClk = 0; iClk < t3DFirmwareInfo.shape()[1]; iClk++) {
      //cout<<iClk<<" 2d: "<<t3DFirmwareInfo[iTrack][iClk][0]<<" tsf: "<<t3DFirmwareInfo[iTrack][iClk][2]<<" evt: "<<t3DFirmwareInfo[iTrack][iClk][3]<<" isOld: "<<t3DFirmwareInfo[iTrack][iClk][1]<<endl;
      // 2DValid
      if (t3DFirmwareInfo[iTrack][iClk][0] == 0) continue;
      //cout<<iClk<<" 2d: "<<t3DFirmwareInfo[iTrack][iClk][0]<<" tsf: "<<t3DFirmwareInfo[iTrack][iClk][2]<<" evt: "<<t3DFirmwareInfo[iTrack][iClk][3]<<" isOld: "<<t3DFirmwareInfo[iTrack][iClk][1]<<endl;
      // TSFValid
//t3DFirmwareInfo[iTrack][iClk][0] (t3d_fnf) := t2d_fnf & TSFValid, so using t3d_fnf is alreay sufficient
//      if (t3DFirmwareInfo[iTrack][iClk][2] == 0) continue;
      // EventTimeValid
//      if (t3DFirmwareInfo[iTrack][iClk][3] == 0) continue;
      // TODO make an algorithm to follow the track.
      // Start follow status of track.
      //choose new tracks
//      if (t3DInfo.size() != 0 && t3DFirmwareInfo[iTrack][iClk][1] == 1) continue;
      //if (t3DFirmwareInfo[iTrack][iClk][1] == 1) continue;
      double track3D_ref[7] = {t3DFirmwareInfo[iTrack][iClk][4], t3DFirmwareInfo[iTrack][iClk][5], t3DFirmwareInfo[iTrack][iClk][6], t3DFirmwareInfo[iTrack][iClk][7], t3DFirmwareInfo[iTrack][iClk][8], t3DFirmwareInfo[iTrack][iClk][9], t3DFirmwareInfo[iTrack][iClk][10]};

      multi_array_ref<double, 1> track3D((double*)track3D_ref, extents[7]);

      t3DInfo.resize(extents[iTrack_filter + 1][7]);
      t3DInfo[iTrack_filter] = track3D;
      iTrack_filter += 1;
    }
  }
}

// t3DInfo[eventTime, charge, rho, phi0, z0, cot, zchi]
void TRGCDCT3DConverterModule::add3DDatastore(multi_array<double, 2>& t3DInfo, bool doConvert)
{
  for (unsigned iTrack = 0; iTrack < t3DInfo.shape()[0]; ++iTrack) {
    double charge = 0, phi0_i = 0, omega = 0, chi2D = 0, z0 = 0, cot = 0, zchi2 = 0;
    double phi0_c = 0;
    if (doConvert) {
      charge = t3DInfo[iTrack][1] == 2 ? -1 : 1;
      phi0_c = t2DPhiTot3DPhi(t3DInfo[iTrack][3], t3DInfo[iTrack][2]) / (pow(2, 12) - 0.5) * M_PI;
      //phi0_i = phi0_c + charge * M_PI_2;
      phi0_i = (45 + 90. / 80 * (1 + t3DInfo[iTrack][3])) / 180 * M_PI;
      //omega = charge / t2DRhoTot3DRho(t3DInfo[iTrack][2]) / (pow(2, 11) - 0.5) * 2500;
      omega = toSigned(t3DInfo[iTrack][2], 7) / 33.0 * 3.2;
      z0 = toSigned(t3DInfo[iTrack][4], 11) * 0.0382;
      cot = toSigned(t3DInfo[iTrack][5], 11) * 0.00195;
      zchi2 = t3DInfo[iTrack][6];
    } else {
      charge = t3DInfo[iTrack][1] == 1 ? 1 : -1;
      phi0_c = t2DPhiTot3DPhi((t3DInfo[iTrack][3] - 34 / 90.*80 - 1), t3DInfo[iTrack][2]) / (pow(2, 12) - 0.5) * M_PI;
      phi0_i = t3DInfo[iTrack][3];
      omega = t3DInfo[iTrack][2];
      z0 = t3DInfo[iTrack][4];
      cot = t3DInfo[iTrack][5];
      zchi2 = t3DInfo[iTrack][6];
    }

    //Correct phi0_i.  The zero points of raw phi0_i for four boards are different.
    if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore0") {
      phi0_c = phi0_c + (M_PI / 2.0) * 0;
      phi0_i = phi0_i + (M_PI / 2.0) * 0;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore1") {
      phi0_c = phi0_c + (M_PI / 2.0) * 1;
      phi0_i = phi0_i + (M_PI / 2.0) * 1;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore2") {
      phi0_c = phi0_c + (M_PI / 2.0) * 2;
      phi0_i = phi0_i + (M_PI / 2.0) * 2;
    } else if (m_firmwareResultCollectionName == "TRGCDCT3DUnpackerStore3") {
      phi0_c = phi0_c + (M_PI / 2.0) * 3;
      phi0_i = phi0_i + (M_PI / 2.0) * 3;
    }
    // Confine the range of the angle to -pi ~ pi
    while (phi0_c > M_PI) {
      phi0_c -= 2 * M_PI;
    }
    while (phi0_i < -M_PI) {
      phi0_c += 2 * M_PI;
    }
    while (phi0_i > M_PI) {
      phi0_i -= 2 * M_PI;
    }
    while (phi0_i < -M_PI) {
      phi0_i += 2 * M_PI;
    }

    //cout<<"iTrack: "<<iTrack<<" eventTime:"<<t3DInfo[iTrack][0]<<" charge:"<<charge<<" omega: "<<omega<<" rho: "<<charge/omega<<" pt: "<<charge/omega*0.3*1.5*0.01<<" phi0_i: "<<phi0_i<<" "<<phi0_i*180/M_PI<<" phi0_c: "<<phi0_c<<" "<<phi0_c*180/M_PI<<" z0:"<<z0<<" cot:"<<cot<<" theta: "<<M_PI/2-atan(cot)<<" zchi:"<<zchi2<<endl;
    if (m_isVerbose) cout << "[3D] iTrack:" << iTrack << " charge: " << charge << " phi0_i:" << phi0_i << " " << phi0_i * 180 / M_PI <<
                            " omega:" << omega << " pt:" << charge / omega * 0.3 * 1.5 * 0.01 << " z0:" << z0 << " cot:" << cot << " zchi2:" << zchi2 << endl;
    m_tracks3D.appendNew(phi0_i, omega, chi2D, z0, cot, zchi2);
  }
}
