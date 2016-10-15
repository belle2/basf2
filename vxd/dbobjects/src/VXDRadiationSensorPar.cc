/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDRadiationSensorPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDRadiationSensorPositionPar::read(const GearDir& position)
{
  m_posZ = position.getLength("z");
  m_radius = position.getLength("radius");
  m_theta = position.getAngle("theta");

  //loop over all phi positions
  for (GearDir& sensor : position.getNodes("phi")) {
    //we need angle and Id
    m_ids.push_back(sensor.getInt("@id"));
    m_phis.push_back(sensor.getAngle());
  }
}

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDRadiationSensorPar::read(const GearDir& content)
{
  m_insideEnvelope = content.getBool("insideEnvelope");
  m_width = content.getLength("width");
  m_length = content.getLength("length");
  m_height = content.getLength("height");
  m_material = content.getString("material");

  //loop over all positions
  for (GearDir& position : content.getNodes("position")) {
    m_sensors.push_back(VXDRadiationSensorPositionPar(position));
  }
}

