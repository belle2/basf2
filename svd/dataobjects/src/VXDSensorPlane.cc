/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dataobjects/VXDSensorPlane.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

ClassImp(VXDSensorPlane)

void VXDSensorPlane::Print(const Option_t* option) const
{
  B2INFO("VXDSensorPlane for " << (string)VxdID(m_sensorID));
}
