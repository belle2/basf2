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

ClassImp(CDCHit)

CDCHit::CDCHit(unsigned short tdcCount, unsigned short charge,
               unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire)
{
  setTDCCount(tdcCount);
  setADCCount(charge);
  setWireID(iSuperLayer, iLayer, iWire);
}
