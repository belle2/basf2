/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/translators/SimpleDriftTimeTranslator.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

float SimpleDriftTimeTranslator::getDriftLength(short driftTime,
                                                const WireID& wireID,
                                                unsigned short timeOfFlightEstimator,
                                                bool,
                                                float z, float)
{
  // Need to undo everything the simple digitization does in reverse order.
  // First: Undo propagation in wire, if it was used:
  if (m_useInWirePropagationDelay) {
    CDCGeometryPar& geometryPar = CDCGeometryPar::Instance();
    m_backWirePos =   geometryPar.wireBackwardPosition(wireID);
    //subtract distance divided by speed of electric signal in the wire from the drift time.
    driftTime -= (z - m_backWirePos.Z()) / 27.25;
  }

  // Second: correct for event time. If this wasn't simulated, m_eventTime can just be set to 0.
  driftTime -= m_eventTime;

  //Third: If time of flight was simulated, this has to be undone, too. If it wasn't timeOfFlightEstimator should be taken as 0.
  driftTime -= timeOfFlightEstimator;

  //Now we have an estimate for the time it took from the ionisation to the hitting of the wire.
  //Need to reverse calculate the relation between drift lenght and drift time.
  return (driftTime * 4e-4);
}

float SimpleDriftTimeTranslator::getDriftLengthResolution(float,
                                                          const WireID& ,
                                                          bool,
                                                          float, float)
{
  return 1e-4; // 100um **2 in cm**2
}
