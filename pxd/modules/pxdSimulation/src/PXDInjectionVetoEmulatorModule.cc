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
           "Time window lower edge for PXD in nano seconds", -10000.0);
  addParam("maxTimePXD", m_maxTimePXD,
           "Time window upper edge for PXD in nano seconds", 0.0);
  addParam("noiseBunchRevolutionTime",
           m_revolutionTime, "Noise bunch revolution time in ns", 10000.0);
  addParam("pxdGatedModeLumiFraction",
           m_pxdGatedModeLumiFraction, "Fraction of time in the PXD Gated Mode for the PXD readout", 0.18);

}

void PXDInjectionVetoEmulatorModule::initialize()
{
  //Register output collections
  m_storePXDIBTiming.registerInDataStore(m_PXDIBTimingName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);

  // Number of PXD gates
  m_nGates = 192;
}

void PXDInjectionVetoEmulatorModule::event()
{
  // Sample a triggergate for this event
  int triggerGate  = gRandom->Integer(m_nGates);

  // Sample the gating flag for this event
  bool pxdGatedModeEvent = false;
  if (gRandom->Rndm() < m_pxdGatedModeLumiFraction) {pxdGatedModeEvent = true;}

  vector<float> gatingStartTimes;

  if (pxdGatedModeEvent) {
    double gatingTime1 = gRandom->Rndm() * (m_maxTimePXD - m_minTimePXD) + m_minTimePXD;
    gatingStartTimes.push_back(gatingTime1);

    double gatingTime2 = gatingTime1 +  m_revolutionTime;
    gatingStartTimes.push_back(gatingTime2);
  }

  m_storePXDIBTiming.create();
}
