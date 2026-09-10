/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <framework/core/Environment.h>
#include <framework/geometry/B2Vector3.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

RealisticTDCCountTranslator::RealisticTDCCountTranslator(bool useInWirePropagationDelay, bool fromTrackCreator) :
  m_useInWirePropagationDelay(useInWirePropagationDelay), m_fromTrackCreator(fromTrackCreator),
  m_gcp(CDCGeoControlPar::getInstance()),
  m_scp(CDCSimControlPar::getInstance()), m_cdcp(CDCGeometryPar::Instance()),
  m_tdcBinWidth(m_cdcp.getTdcBinWidth())
{
  m_realData = !Environment::Instance().isMC();
  if (m_realData) {
    m_fudgeFactor = m_cdcp.getFudgeFactorForSigma(0);
  } else {
    m_fudgeFactor = m_cdcp.getFudgeFactorForSigma(1);
  }
}


double RealisticTDCCountTranslator::getDriftTime(unsigned short tdcCount,
                                                 const WireID& wireID,
                                                 double timeOfFlightEstimator,
                                                 double z,
                                                 unsigned short adcCount)
{
  // translate TDC Count into time information:
  // N.B. No correction (+ or -0.5 count) is needed in the translation since no bias is in the real TDC count on average (info. from KEK electronics division).
  double driftTime = m_cdcp.getT0(wireID) - tdcCount * m_tdcBinWidth;

  // cppcheck-suppress variableScope ; kept next to the related declarations for readability
  unsigned short layer = wireID.getICLayer();

  // Need to undo everything the digitization does in reverse order.
  // First: Undo propagation in wire, if it was used:
  if (m_useInWirePropagationDelay) {
    const B2Vector3D& backWirePos = m_cdcp.wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
    const B2Vector3D& diffWirePos = m_cdcp.wireForwardPosition(wireID, CDCGeometryPar::c_Aligned) - backWirePos;
    // subtract distance divided by speed of electric signal in the wire from the drift time.
    double propLength = z - backWirePos.Z();
    double dZ = diffWirePos.Z();
    if (dZ != 0.) {
      propLength *= diffWirePos.Mag() / dZ;
    }
    if (m_gcp.getSenseWireZposMode() == 1) {
      propLength += m_cdcp.getBwdDeltaZ(layer);
    }
    driftTime -= propLength * m_cdcp.getPropSpeedInv(layer);
  }

  // Second: correct for event time. If this wasn't simulated, m_eventTime can just be set to 0.
  // Use SVD temporary T0 preferentially, falling back to CDC T0, then getEventT0().
  // SVD and CDC temporary entries are set before any TrackCreator runs and remain stable across
  // multiple fitting passes, unlike getEventT0() which can change when EventT0Combiner runs between
  // passes. The fallback to getEventT0() is there in case neither SVD nor CDC proved event T0.
  if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
    driftTime -= m_eventTimeStoreObject->getEventT0(m_fromTrackCreator);
  }

  //Third: If time of flight was simulated, this has to be undone, too. If it wasn't timeOfFlightEstimator should be taken as 0.
  driftTime -= timeOfFlightEstimator;

  //Forth: Time-walk correction
  if (m_realData) { //for data, always correct
    driftTime -= m_cdcp.getTimeWalk(wireID, adcCount);
  } else if (m_scp.getTimeWalk()) { //for MC, ccorrect if the flag is on
    driftTime -= m_cdcp.getTimeWalk(wireID, adcCount);
  }
  return driftTime;
}


double RealisticTDCCountTranslator::getDriftLength(unsigned short tdcCount,
                                                   const WireID& wireID,
                                                   double timeOfFlightEstimator,
                                                   bool leftRight,
                                                   double z,
                                                   double alpha,
                                                   double theta,
                                                   unsigned short adcCount)
{
  const double driftTime = getDriftTime(tdcCount, wireID, timeOfFlightEstimator, z, adcCount);

  unsigned short layer = wireID.getICLayer();

  // Now we have an estimate for the time it took from the ionisation to the hitting of the wire.
  // Need to reverse calculate the relation between drift length and drift time.
  double driftL = m_cdcp.getDriftLength(driftTime, layer, leftRight, alpha, theta);

  return driftL;
}


/** this function returns the variance that is used as the CDC measurement resolution in track fitting */
double RealisticTDCCountTranslator::getDriftLengthResolution(double driftLength, const WireID&  wireID, bool leftRight, double z,
    double alpha, double theta)
{
  static_cast<void>(z); //just to suppress warning of unused
  double resol = m_fudgeFactor * m_cdcp.getSigma(driftLength, wireID.getICLayer(), leftRight, alpha, theta);

  return resol * resol;;
}
