/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/SensorPlane.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void VXD::SensorPlane::Print(const Option_t*) const
{
  B2INFO("VXDSensorPlane for " << (string)VxdID(m_sensorID));
}
