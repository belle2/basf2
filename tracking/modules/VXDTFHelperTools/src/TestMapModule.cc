/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/VXDTFHelperTools/TestMapModule.h"
#include "tracking/vxdCaTracking/LittleHelper.h"
#include "tracking/vxdCaTracking/FullSecID.h"
#include <framework/logging/Logger.h>
#include <boost/foreach.hpp>
#include <algorithm>
// #include <functional>
// #include <utility>
#include <iterator>     // std::distance
#include <math.h>       /* floor */


using namespace std;
using namespace Belle2;
using namespace Tracking;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TestMap)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestMapModule::TestMapModule() : Module()
{
  //Set module properties
  setDescription("tests std::map for sectorMaps");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("numLayers", m_PARAMnumLayers, "sets how many layers are assumed (useful value: 4-6)", int(6));
  addParam("numSensors", m_PARAMnumSensors, "sets how many sensors per layer are assumed (useful value: ~227/numLayers)", int(38));
  addParam("numSectors", m_PARAMnumSectors, "sets how many sectors per sensor are assumed (useful value: 4-50)", int(15));
  addParam("numFriends", m_PARAMnumFriends, "sets how many friends per sector are assumed (useful value: 2-20)", int(3));
}


TestMapModule::~TestMapModule()
{

}


void TestMapModule::initialize()
{
  m_sectorAdresses.clear();
  FillVector(m_sectorAdresses);
  JumbleVector(m_sectorAdresses);
  FillMaps();
}


void TestMapModule::beginRun()
{
  m_eventCounter = 0;
}


void TestMapModule::event()
{
  m_eventCounter++;
  if (m_eventCounter % 500 == 0) {
    B2INFO("TestMapModule - Event: " << m_eventCounter);
  }
  LittleHelper littleHelperBox = LittleHelper();

  int rndAccess = littleHelperBox.getRandomIntegerGauss(50, 10);
  int numSectors = m_sectorAdresses.size();
  if (rndAccess < 0) { rndAccess = -rndAccess; }
  int rndSector;

  vector<int> accessVariables;
  for (int i = 0; i < rndAccess; ++i) {
    rndSector = littleHelperBox.getRandomIntegerUniform(0, numSectors);
    accessVariables.push_back(rndSector);
  }

  boostClock::time_point beginEvent = boostClock::now();
  BOOST_FOREACH(int sectorValue, accessVariables) {
//    const vector<unsigned int>& friends = m_testMap.find(m_sectorAdresses[sectorValue])->second.getFriends();
    unsigned int secID = m_testMap.find(m_sectorAdresses[sectorValue])->second.getSecID();
    ++secID;
  }
  boostClock::time_point stopTimer = boostClock::now();
  m_eventMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  /*boostClock::time_point */beginEvent = boostClock::now();
  BOOST_FOREACH(int sectorValue, accessVariables) {
//    const vector<unsigned int>& friends = m_testFlatMap.find(m_sectorAdresses[sectorValue])->second.getFriends();
    unsigned int secID = m_testFlatMap.find(m_sectorAdresses[sectorValue])->second.getSecID();
    ++secID;
  }
  /*boostClock::time_point */stopTimer = boostClock::now();
  m_eventFlatMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  /*boostClock::time_point */beginEvent = boostClock::now();
  BOOST_FOREACH(int sectorValue, accessVariables) {
    unsigned int secID = m_testUnorderedMap.find(m_sectorAdresses[sectorValue])->second.getSecID();
    ++secID;
  }
  /*boostClock::time_point */stopTimer = boostClock::now();
  m_eventUnorderedMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

}


void TestMapModule::endRun()
{
}


void TestMapModule::terminate()
{

  B2WARNING("std::map-Filling took " << (m_fillMapStuff.count()) << " nanoseconds")
  B2WARNING("boost::flat_map-Filling took " << (m_fillFlatMapStuff.count()) << " nanoseconds")
  B2WARNING("boost::unordered_map-Filling took " << (m_fillUnorderedMapStuff.count()) << " nanoseconds")
  B2WARNING("std::map-Reading took " << (m_eventMapStuff.count()) << " nanoseconds")
  B2WARNING("boost::flat_map-Reading took " << (m_eventFlatMapStuff.count()) << " nanoseconds")
  B2WARNING("boost::unordered_map-Reading took " << (m_eventUnorderedMapStuff.count()) << " nanoseconds")
}

void TestMapModule::FillMaps()
{
  LittleHelper littleHelperBox = LittleHelper();

  int numSectors = m_sectorAdresses.size();
  vector<VXDSector> sectors, seccopy;
  BOOST_FOREACH(unsigned int fullSecID, m_sectorAdresses) { // produce sectors
    VXDSector aSector = VXDSector(fullSecID);
    for (int i = 0; i < m_PARAMnumFriends; ++i) {
      aSector.addFriend(littleHelperBox.getRandomIntegerUniform(0, numSectors));
    }
    sectors.push_back(aSector);
  } // is done in a separate loop to get better time resolution of the fillin-process
  seccopy = sectors;

  std::sort(seccopy.begin(), seccopy.end());
  vector<VXDSector>::iterator newEndOfVector;
  newEndOfVector = std::unique(seccopy.begin(), seccopy.end()); /// WARNING: std:unique does delete double entries but does NOT resize the vector! This means that for every removed element, at the end of the vector remains one random value stored
  int vSize = std::distance(seccopy.begin(), newEndOfVector);
  seccopy.resize(vSize);
  B2INFO("TestMapModule::FillMaps: m_sectorAdresses has " << m_sectorAdresses.size() << ", sectors has " << sectors.size() << ", seccopy has " << seccopy.size() << " entries...")
  // stl-map
  vector<VXDSector>::iterator secIt = sectors.begin();
  boostClock::time_point beginEvent = boostClock::now();
  for (; secIt < sectors.end(); ++secIt) {
    m_testMap.insert(make_pair(secIt->getSecID(), (*secIt)));
  }
  boostClock::time_point stopTimer = boostClock::now();
  m_fillMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  //boost-flat_map
  secIt = sectors.begin();
  /*boostClock::time_point */beginEvent = boostClock::now();
  for (; secIt < sectors.end(); ++secIt) {
    m_testFlatMap.insert(make_pair(secIt->getSecID(), (*secIt)));
  }
  /*boostClock::time_point */stopTimer = boostClock::now();
  m_fillFlatMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  //boost-unordered_map
  secIt = sectors.begin();
  /*boostClock::time_point */beginEvent = boostClock::now();
  for (; secIt < sectors.end(); ++secIt) {
    m_testUnorderedMap.insert(make_pair(secIt->getSecID(), (*secIt)));
  }
  /*boostClock::time_point*/ stopTimer = boostClock::now();
  m_fillUnorderedMapStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  B2INFO("TestFlatMapModule::FillMap: map has " << m_testMap.size() << "/" << m_testFlatMap.size() << "/" << m_testUnorderedMap.size() << " entries...")
}/*{
  LittleHelper littleHelperBox = LittleHelper();

  int numSectors = m_sectorAdresses.size();
  BOOST_FOREACH(unsigned int fullSecID, m_sectorAdresses) {

    VXDSector aSector = VXDSector(fullSecID);
    for (int i = 0; i < m_PARAMnumFriends; ++i) {
      aSector.addFriend(littleHelperBox.getRandomIntegerUniform(0, numSectors));
    }

    boostClock::time_point beginEvent = boostClock::now();
    m_testMap.insert(make_pair(fullSecID, aSector));
    boostClock::time_point stopTimer = boostClock::now();
    m_fillStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
  }

  B2INFO("TestMapModule::FillMap: map has " << m_testMap.size() << " entries...")
}*/

void TestMapModule::FillVector(vector<unsigned int>& vec)
{
  for (int lay = 1; lay <= m_PARAMnumLayers; ++lay) {
    bool slay = true;
    for (int sen = 0; sen < m_PARAMnumSensors; ++sen) {
      for (int sec = 0; sec < m_PARAMnumSectors; ++sec) {
        vec.push_back(FullSecID(lay, slay, sen, sec).getFullSecID());
      }
    }
    slay = false;
    for (int sen = 0; sen < m_PARAMnumSensors; ++sen) {
      for (int sec = 0; sec < m_PARAMnumSectors; ++sec) {
        vec.push_back(FullSecID(lay, slay, sen, sec).getFullSecID());
      }
    }
  }
  std::sort(vec.begin(), vec.end());
  for (int pos = 1; pos < int(vec.size()); ++pos) {
    if (vec[pos] == vec[pos - 1]) {
      B2INFO(" vecPos " << pos << " double entry, FullSecID/string: " << vec[pos] << "/" << FullSecID(vec[pos]).getFullSecString())
    }
  }

  B2INFO("TestMapModule::FillVector: vector has " << vec.size() << " entries...")
}

void TestMapModule::JumbleVector(vector<unsigned int>& vec)
{
  LittleHelper littleHelperBox = LittleHelper();
  int index1, index2, entry, length = vec.size();
  for (int i = 0; i < length; i++) {
    index1 = littleHelperBox.getRandomIntegerUniform(0, length);
    index2 = littleHelperBox.getRandomIntegerUniform(0, length);
    entry = vec[index1];
    vec[index1] = vec[index2];
    vec[index2] = entry;
  }
}