/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/TcFourHitFilters.h"
#include "../include/Cutoff.h"

using namespace std;
using namespace Belle2;


bool TcFourHitFilters::checkDeltapT(string nameDeltapT)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltapT, m_friendID);
  if (pCutoff != NULL) {
    double aResult = FourHitFilters::deltapT();
    if (/*aResult > pCutoff->getMinValue() &&*/ aResult < pCutoff->getMaxValue()) {
      return true;
    } else { return false; }
  } else { return false; }
}


bool TcFourHitFilters::checkDeltaDistCircleCenter(string nameDeltaDistCircleCenter)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaDistCircleCenter, m_friendID);
  if (pCutoff != NULL) {
    double aResult = FourHitFilters::deltaDistCircleCenter();
    if (aResult < pCutoff->getMaxValue()) {
      return true;
    } else { return false; }
  } else { return false; }
}


pair <double, double> TcFourHitFilters::getCutoffs(string aFilter)
{
  pair <double, double> pair;
  Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) {
    pair = make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
    return pair;;
  } else { return make_pair(0., 0.); }
}
