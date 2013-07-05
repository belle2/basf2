/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SegFinderFilters.h"
#include "../include/Cutoff.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;

bool SegFinderFilters::checkDist3D(int nameDist3D)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDist3D();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      m_dist3DCtr.first++;
      return true;
    }
  }
  m_dist3DCtr.second++;
  return false;
}

bool SegFinderFilters::checkDistXY(int nameDistXY)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDistXY, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistXY();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      m_distXYCtr.first++;
      return true;
    }
  }
  m_distXYCtr.second++;
  return false;
}

bool SegFinderFilters::checkDistZ(int nameDistZ)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDistZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      m_distZCtr.first++;
      return true;
    }
  }
  m_distZCtr.second++;
  return false;
}

bool SegFinderFilters::checkNormedDist3D(int nameNormDist3D)    // max only
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameNormDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcNormedDist3D();
    if (aResult < pCutoff->getMaxValue()) {
      m_normDist3DCtr.first++;
      return true;
    }
  }
  m_normDist3DCtr.second++;
  return false;
}

bool SegFinderFilters::checkSlopeRZ(int nameSlopeRZ)    // min & max
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameSlopeRZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcSlopeRZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_slopeRZCtr.first++;
      return true;
    }
  }
  m_slopeRZCtr.second++;
  return false;
}


pair <double, double> SegFinderFilters::getCutoffs(int aFilter)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) {
//     pair <double, double> aPair = make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
    return make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
  }
  return make_pair(0., 0.);
}
