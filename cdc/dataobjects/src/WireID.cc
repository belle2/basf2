/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/dataobjects/WireID.h"

using namespace Belle2;
using namespace std;

ClassImp(WireID)

void WireID::setWireID(const unsigned short iCLayer, const unsigned short iWire)
{
  B2DEBUG(250, "setWireID called with " << iCLayer << ", " << iWire);
  if (iCLayer < 8) {
    setWireID(0, iCLayer, iWire);
  } else {
    setWireID(((iCLayer - 8) / 6) + 1, (iCLayer - 2) % 6, iWire);
  }
}

unsigned short WireID::getICLayer() const
{
  if (getISuperLayer() == 0) { return getILayer(); }
  return 8 + (getISuperLayer() - 1) * 6 + getILayer();
}

WireID::WireID(const unsigned short iCLayer, const unsigned short iWire)
{
  setWireID(iCLayer, iWire);
}

WireID::WireID(const unsigned short iSuperLayer, const unsigned short iLayer, const unsigned short iWire)
{
  setWireID(iSuperLayer, iLayer, iWire);
}
