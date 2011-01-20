/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;

ClassImp(CDCHit)

CDCHit::CDCHit(const double& driftTime, const double& charge,
               const int& iSuperLayer, const int& iLayer, int iWire)
{
  B2INFO("Complexer Konstruktor aufgerufen.")
  setDriftTime(driftTime);
  setCharge(charge);
  setWireId(iSuperLayer, iLayer, iWire);
}
