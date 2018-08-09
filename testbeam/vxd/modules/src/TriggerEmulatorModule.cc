/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>

#include <vxd/dataobjects/VxdID.h>

#include <testbeam/vxd/modules/TriggerEmulatorModule.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TriggerEmulator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TriggerEmulatorModule::TriggerEmulatorModule() : Module()
{
  // Set module properties
  setDescription("Sets the module's return value to the number of planes hit (of the planes specified) which can be used to split the analysis path. Currently only supports telescope hits.");

  // Parameter definitions
  addParam("sensorList", m_sensors, "the VxdIDs of all sensors seperated by spaces which should be used for the trigger",
           std::string(""));
  addParam("inputArrayName", m_inputArrayName, "the name of the StoreArray with the telescope (and Scintilator) clusters",
           std::string(""));

}

TriggerEmulatorModule::~TriggerEmulatorModule()
{
}

void TriggerEmulatorModule::initialize()
{
  StoreArray<TelCluster> TelCluster(m_inputArrayName);  TelCluster.isRequired();
}

void TriggerEmulatorModule::beginRun()
{
}

void TriggerEmulatorModule::event()
{
  // return 0 if no hits have been found
  setReturnValue(0);


  StoreArray<TelCluster> telclusters(m_inputArrayName);

  std::map<VxdID, int> counter;
  for (TelCluster& acluster : telclusters) {
    // if the clusters VxdId id contained in the list of sensors do the counting
    VxdID id = acluster.getSensorID();
    if (m_sensors.find((std::string)id) != std::string::npos) {
      B2DEBUG(1, "Found the senor " << (std::string)id << " in the list of sensors ");
      counter[id]++;
    }
  }

  //the number of entries in the map gives the number of sensors with at least one cluster
  setReturnValue((int)counter.size());


}

void TriggerEmulatorModule::endRun()
{
}

void TriggerEmulatorModule::terminate()
{
}


