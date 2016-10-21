/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeoSensorPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
/*
void VXDGeoSensorPar::read(const GearDir& paramsSensor)
{
  m_material = paramsSensor.getString("Material");
  m_color = paramsSensor.getString("Color", "");
  m_width = paramsSensor.getLength("width");
  m_width2 = paramsSensor.getLength("width2", 0);
  m_lenght = paramsSensor.getLength("length");
  m_height = paramsSensor.getLength("height");
  m_slanted = paramsSensor.getBool("@slanted", false);
}
*/




