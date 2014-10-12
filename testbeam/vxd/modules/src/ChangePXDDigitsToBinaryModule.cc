/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/ChangePXDDigitsToBinaryModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ChangePXDDigitsToBinary)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ChangePXDDigitsToBinaryModule::ChangePXDDigitsToBinaryModule() : Module(), m_binaryValue(2000.), m_threshold(700.)
{
  // Set module properties
  setDescription("Changes specified PXD sensors to binary readout to emulate EUDET telescopes. Must be run before PXDClusterizer");
  // List of default sensorIDs for EUDET telescopes in geometry XML
  std::vector<std::string> defaultEUDETids;
  defaultEUDETids.push_back("7.2.1");
  defaultEUDETids.push_back("7.2.2");
  defaultEUDETids.push_back("7.2.3");
  defaultEUDETids.push_back("7.3.4");
  defaultEUDETids.push_back("7.3.5");
  defaultEUDETids.push_back("7.3.6");
  // Parameter definitions
  addParam("BinaryValue", m_binaryValue, "Charge for pixel ON", m_binaryValue);
  addParam("Threshold", m_threshold, "Minimum charge for pixel to be ON", m_threshold);
  addParam("SensorIDs", m_sensorIDs, "List of sensorIDs to be changed to binary readout", defaultEUDETids);
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
}

void ChangePXDDigitsToBinaryModule::initialize()
{
  StoreArray<PXDDigit> PXDDigits(m_storeDigitsName);
  PXDDigits.isRequired();
  PXDDigits.registerInDataStore();
  // Store actuall name of the collection
  m_storeDigitsName = PXDDigits.getName();
  // Convert string representation of ids into VxdIDs
  for (std::string id : m_sensorIDs)
    m_sensorIDsVXD.push_back(VxdID(id));
}

void ChangePXDDigitsToBinaryModule::event()
{
  bool toBinary = false;
  VxdID lastID(0);

  StoreArray<PXDDigit> PXDDigits(m_storeDigitsName);

  for (PXDDigit & digit : PXDDigits) {
    // Faster if digits are sorted by sensor...
    if (digit.getSensorID() != lastID) {
      // Is sensor of the digit in the list?
      if (std::find(m_sensorIDsVXD.begin(), m_sensorIDsVXD.end(), digit.getSensorID()) != m_sensorIDsVXD.end())
        toBinary = true;
      else
        toBinary = false;
    }

    if (toBinary) {
      if (digit.getCharge() > m_threshold)
        digit.setCharge(m_binaryValue);
      else
        digit.setCharge(0.);
    }

    lastID = digit.getSensorID();
  }
}



