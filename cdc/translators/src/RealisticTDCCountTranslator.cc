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

float RealisticTDCCountTranslator::getDriftLength(unsigned short tdcCount,
                                                  const WireID& wireID,
                                                  float timeOfFlightEstimator,
                                                  bool leftRight,
                                                  float z,
                                                  float alpha,
                                                  float theta,
                                                  unsigned short adcCount)
{
  // translate TDC Count into time information:
  // N.B. 0.5 is necessary since real TDC module rounds down the time.
  //  float driftTime = (static_cast<float>(m_tdcOffset - (tdcCount + 0.5))) * m_tdcBinWidth;
  float driftTime = m_cdcp.getT0(wireID) - (tdcCount + 0.5) * m_tdcBinWidth;
  //  std::cout << "t0= " << m_cdcp.getT0(wireID) << std::endl;

  unsigned short layer = wireID.getICLayer();

  // Need to undo everything the digitization does in reverse order.
  // First: Undo propagation in wire, if it was used:
  if (m_useInWirePropagationDelay) {
    //    m_backWirePos = m_cdcp.wireBackwardPosition(wireID);
    m_backWirePos = m_cdcp.wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
    //subtract distance divided by speed of electric signal in the wire from the drift time.
    driftTime -= (z - m_backWirePos.Z()) * m_cdcp.getPropSpeedInv(layer);
  }

  // Second: correct for event time. If this wasn't simulated, m_eventTime can just be set to 0.
  driftTime -= m_eventTime;

  //Third: If time of flight was simulated, this has to be undone, too. If it wasn't timeOfFlightEstimator should be taken as 0.
  driftTime -= timeOfFlightEstimator;

  //Forth: Time-walk correction
  driftTime -= m_cdcp.getTimeWalk(wireID, adcCount);

  //Now we have an estimate for the time it took from the ionisation to the hitting of the wire.
  //Need to reverse calculate the relation between drift lenght and drift time.
  float driftL = (driftTime >= 0.) ? m_cdcp.getDriftLength(driftTime, layer, leftRight, alpha,
                                                           theta) : m_vFactor * driftTime;

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

float RealisticTDCCountTranslator::getDriftLengthResolution(float driftLength,
                                                            const WireID&  wireID,
                                                            bool,
                                                            float,
                                                            float,
                                                            float)
{
  float resol = m_cdcp.getSigma(driftLength, wireID.getICLayer());

#if defined(CDC_DEBUG)
  cout << " " << endl;
  cout << "RealisticTDCCountTranslator::getDriftLengthResolution" << endl;
  cout << "spaceResol= " << resol << endl;
#endif

  return resol * resol;;
}
