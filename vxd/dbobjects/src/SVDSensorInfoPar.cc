/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDSensorInfoPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDSensorInfoPar::read(const GearDir& sensor)
{
  m_width = sensor.getLength("width");
  m_length = sensor.getLength("length");
  m_height = sensor.getLength("height");
  m_stripsU = sensor.getInt("stripsU");
  m_stripsV = sensor.getInt("stripsV");
  m_width2 = sensor.getLength("width2", 0);

  m_depletionVoltage = sensor.getWithUnit("DepletionVoltage");
  m_biasVoltage = sensor.getWithUnit("BiasVoltage");
  m_backplaneCapacitance = sensor.getDouble("BackplaneCapacitance");
  m_interstripCapacitance = sensor.getDouble("InterstripCapacitance");
  m_couplingCapacitance = sensor.getDouble("CouplingCapacitance");
  m_electronicNoiseU = sensor.getWithUnit("ElectronicNoiseU");
  m_electronicNoiseV = sensor.getWithUnit("ElectronicNoiseV");
}
