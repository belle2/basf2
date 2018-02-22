/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/ThrustOfEvent.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

void ThrustOfEvent::addThrustAxis(TVector3 thrustAxis)
{
  m_thrustAxis = thrustAxis;
}

void ThrustOfEvent::addThrust(float thrust)
{
  m_thrust = thrust;
}
