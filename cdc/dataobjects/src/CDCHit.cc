/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;

CDCHit::CDCHit(unsigned short tdcCount, unsigned short charge,
               unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire, unsigned short tdcCount2ndHit, unsigned short status)
{
  setTDCCount(tdcCount);
  setADCCount(charge);
  setWireID(iSuperLayer, iLayer, iWire);
  setTDCCount2ndHit(tdcCount2ndHit);
  setStatus(status);
}


DigitBase::EAppendStatus CDCHit::addBGDigit(const DigitBase* bg)
{
  const auto* bgDigit = static_cast<const CDCHit*>(bg);

  int diff  = static_cast<int>(m_tdcCount) - static_cast<int>(bgDigit->getTDCCount());

  // If the BG hit is faster than the true hit, the TDC count is replaced.
  // ADC counts are summed up.
  if (diff < 0) {
    m_tdcCount = bgDigit->getTDCCount();
  }
  m_adcCount += bgDigit->getADCCount();
  return DigitBase::c_DontAppend;

}
