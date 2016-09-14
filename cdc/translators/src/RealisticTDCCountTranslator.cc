/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

RealisticTDCCountTranslator::RealisticTDCCountTranslator(bool useInWirePropagationDelay) :
  m_useInWirePropagationDelay(useInWirePropagationDelay), m_eventTime(0), m_cdcp(CDCGeometryPar::Instance()),
  m_tdcBinWidth(m_cdcp.getTdcBinWidth()), m_vFactor(1.)
{
  //  m_tdcOffset   = m_cdcp.getTdcOffset();
  //  m_tdcBinWidth = m_cdcp.getTdcBinWidth();

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "RealisticTDCCountTranslator constructor" << endl;
  cout << "m_cdcp=" << &m_cdcp << endl;
  cout << "m_tdcBinWidth=" << m_tdcBinWidth << endl;
  cout << "m_vFactor=" << m_vFactor << endl;
#endif
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
  // translate TDC Count into time information:
  // N.B. No correction (+ or -0.5 count) is needed in the translation since no bias is in the real TDC count on average (info. from KEK electronics division).
  double driftTime = m_cdcp.getT0(wireID) - tdcCount * m_tdcBinWidth;
  //  std::cout << "t0,tdcbinw= " << m_cdcp.getT0(wireID) <<" "<< m_tdcBinWidth << std::endl;

  unsigned short layer = wireID.getICLayer();

  // Need to undo everything the digitization does in reverse order.
  // First: Undo propagation in wire, if it was used:
  if (m_useInWirePropagationDelay) {
    //N.B. stereo-angle effect is ignored in the following corr.
    m_backWirePos = m_cdcp.wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
    //subtract distance divided by speed of electric signal in the wire from the drift time.
    double zb = m_backWirePos.Z();
    if (m_cdcp.getSenseWireZposMode() == 1) {
      //      std::cout <<"layer,zb,dzb,zf,dzf= "<< layer <<" "<< zb <<" "<< m_cdcp.getBwdDeltaZ(layer) <<" "<< m_cdcp.wireForwardPosition(wireID, CDCGeometryPar::c_Aligned).Z() <<" "<< m_cdcp.getFwdDeltaZ(layer) << std::endl;
      zb -= m_cdcp.getBwdDeltaZ(layer);
    }
    //    driftTime -= (z - m_backWirePos.Z()) * m_cdcp.getPropSpeedInv(layer);
    driftTime -= (z - zb) * m_cdcp.getPropSpeedInv(layer);
  }

  // Second: correct for event time. If this wasn't simulated, m_eventTime can just be set to 0.
  driftTime -= m_eventTime;

  //Third: If time of flight was simulated, this has to be undone, too. If it wasn't timeOfFlightEstimator should be taken as 0.
  driftTime -= timeOfFlightEstimator;

  //Forth: Time-walk correction
  driftTime -= m_cdcp.getTimeWalk(wireID, adcCount);

  //Now we have an estimate for the time it took from the ionisation to the hitting of the wire.
  //Need to reverse calculate the relation between drift lenght and drift time.
  double driftL = std::copysign(m_cdcp.getDriftLength(fabs(driftTime), layer, leftRight, alpha, theta), driftTime);

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "RealisticTDCCountTranslator::getDriftLength" << endl;
  cout << "driftTime=" << driftTime << endl;
  cout << "layer=" << layer << endl;
  cout << "leftright=" << leftRight << endl;
  cout << "driftL= " << driftL << endl;
#endif

  return driftL;
}


/** this function returns the variance that is used as the CDC measurment resolution in track fitting */

double RealisticTDCCountTranslator::getDriftLengthResolution(double driftLength, const WireID&  wireID, bool leftRight, double z,
    double alpha, double theta)
{
  static_cast<void>(z); //just to suppress warning of unused
  double resol = m_cdcp.getSigma(driftLength, wireID.getICLayer(), leftRight, alpha, theta);

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "RealisticTDCCountTranslator::getDriftLengthResolution" << endl;
  cout << "spaceResol= " << resol << endl;
#endif

  return resol * resol;;
}
