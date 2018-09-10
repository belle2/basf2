/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdSimulation/PXDInjectionVetoEmulatorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/GeoTools.h>

#include <vector>

#include <TRandom3.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDInjectionVetoEmulator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDInjectionVetoEmulatorModule::PXDInjectionVetoEmulatorModule() : Module()
{
  // Set module properties
  setDescription("The module emulates the timing for gated mode operation of PXD.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PXDIBTimingName", m_PXDIBTimingName,
           "The name of the StoreObjPtr of generated Injection Bg timing", std::string(""));
  addParam("minTimePXD", m_minTimePXD,
           "Minimum time for the first passage of a noise bunch that can deposit charge in the PXD in nano seconds", -20000.0);
  addParam("maxTimePXD", m_maxTimePXD,
           "Maximum time for the first passage of a noise bunch that can deposit charge in the PXD in nano seconds", -10000.0);
  addParam("noiseBunchRevolutionTime",
           m_revolutionTime, "Noise bunch revolution time in ns", 10000.0);
  addParam("pxdGatedModeLumiFraction",
           m_pxdGatedModeLumiFraction, "Fraction of events which are affected by PXD gated mode", 0.2);

}

void PXDInjectionVetoEmulatorModule::initialize()
{
  //Register output collections
  m_storePXDIBTiming.registerInDataStore(m_PXDIBTimingName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);

  // Get number of PXD gates
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  m_nGates = gTools->getNumberOfPXDReadoutGates();
}

void PXDInjectionVetoEmulatorModule::event()
{
  m_storePXDIBTiming.create();

  // Sample the TriggerGate for this event
  // TriggerGate is read (sampled-cleared) at t=0
  int triggerGate  = gRandom->Integer(m_nGates);
  m_storePXDIBTiming->setTriggerGate(triggerGate);

  // Sample the gating flag for this event
  bool isGated = false;
  if (gRandom->Rndm() < m_pxdGatedModeLumiFraction) {isGated = true;}
  m_storePXDIBTiming->setGated(isGated);

  // Sample gating start times
  if (isGated) {
    float time = 0;
    // Time when tracks from noise bunches hit PXD
    time = gRandom->Rndm() * (m_maxTimePXD - m_minTimePXD) + m_minTimePXD;
    // PXD accumulates charge from four passges of noisy bunches.
    m_storePXDIBTiming->getGatingStartTimes().push_back(time);
    m_storePXDIBTiming->getGatingStartTimes().push_back(time + m_revolutionTime);
    m_storePXDIBTiming->getGatingStartTimes().push_back(time + 2 * m_revolutionTime);
    m_storePXDIBTiming->getGatingStartTimes().push_back(time + 3 * m_revolutionTime);
  }

}
