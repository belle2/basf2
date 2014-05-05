/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/TcFourHitFilters.h"
#include "tracking/vxdCaTracking/Cutoff.h"

using namespace std;
using namespace Belle2;


bool TcFourHitFilters::checkDeltapT(int nameDeltapT)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltapT, m_friendID);
  if (pCutoff != NULL) {
    double aResult = FourHitFilters::calcDeltapT();
    if (/*aResult > pCutoff->getMinValue() &&*/ aResult < pCutoff->getMaxValue()) {
      m_deltaPtCtr.first++;
      return true;
    }
  }
  m_deltaPtCtr.second++;
  return false;
}


bool TcFourHitFilters::checkDeltaDistCircleCenter(int nameDeltaDistCircleCenter)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaDistCircleCenter, m_friendID);
  if (pCutoff != NULL) {
    double aResult = FourHitFilters::deltaDistCircleCenter();
    if (aResult < pCutoff->getMaxValue()) {
      m_deltaDistCircleCenterCtr.first++;
      return true;
    }
  }
  m_deltaDistCircleCenterCtr.second++;
  return false;
}


pair <double, double> TcFourHitFilters::getCutoffs(int aFilter)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) return make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
  return make_pair(0., 0.);
}
