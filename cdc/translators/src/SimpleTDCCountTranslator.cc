/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

double SimpleTDCCountTranslator::getDriftLength(unsigned short tdcCount,
                                                const WireID& wireID,
                                                double timeOfFlightEstimator,
                                                bool,
                                                double z,
                                                double,
                                                double,
                                                unsigned short)
{
  const double driftTime = getDriftTime(tdcCount, wireID, timeOfFlightEstimator, z, 0);

  //Now we have an estimate for the time it took from the ionisation to the hitting of the wire.
  //Need to reverse calculate the relation between drift lenght and drift time.
  double driftL = (driftTime >= 0.) ? driftTime * 4e-3 : -999.;
  return driftL;
}

double SimpleTDCCountTranslator::getDriftTime(unsigned short tdcCount,
                                              const WireID& wireID,
                                              double timeOfFlightEstimator,
                                              double z,
                                              unsigned short)
{
  // translate TDC Count into time information:
  CDCGeometryPar& geometryPar = CDCGeometryPar::Instance();
  double driftTime = (static_cast<double>(geometryPar.getTdcOffset() - (tdcCount + 0.5))); // 1 Unit in the TDC count equals 1 ns

  // Need to undo everything the simple digitization does in reverse order.
  // First: Undo propagation in wire, if it was used:
  if (m_useInWirePropagationDelay) {
    //    CDCGeometryPar& geometryPar = CDCGeometryPar::Instance();
    //    m_backWirePos =   geometryPar.wireBackwardPosition(wireID);
    m_backWirePos =   geometryPar.wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
    //subtract distance divided by speed of electric signal in the wire from the drift time.
    driftTime -= (z - m_backWirePos.Z()) / 27.25;
  }

  // Second: correct for event time. If this wasn't simulated, m_eventTime can just be set to 0.
  driftTime -= m_eventTime;

  //Third: If time of flight was simulated, this has to be undone, too. If it wasn't timeOfFlightEstimator should be taken as 0.
  driftTime -= timeOfFlightEstimator;

  return driftTime;
}

/** this function returns the variance that is used as the CDC measurment resolution in track fitting
if the default resolution of the CDC Digitizer is changed this value has to be changed, too!
this variance is calculated by sigam_cdcHit^2 + sigma_translationError^2
sigma_translationError is an additional (non-gaussian) smearing is introduced due to driftlength to TDC conversio  and
can be (partially) corrected by adding sigma_translationError^2 = (40/sqrt(12) µm)^2 to sigam_cdcHit^2
see Ozaki's mail: [belle2_tracking:0515] */
double SimpleTDCCountTranslator::getDriftLengthResolution(double,
                                                          const WireID&,
                                                          bool,
                                                          double,
                                                          double,
                                                          double)
{
//  return 0.000169 + 1.3333333E-6; // (130 µm)^2 in cm^2 + (40/sqrt(12) µm)^2 also in cm
  return 0.000169;  //the above should be taken into account in the digitizer, not here
}
