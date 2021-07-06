/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/geometry/SensorPlane.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void VXD::SensorPlane::Print(const Option_t*) const
{
  B2INFO("VXDSensorPlane for " << (string)VxdID(m_sensorID));
}
