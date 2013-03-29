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

bool SegFinderFilters::checkDist3D(std::string nameDist3D)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDist3D();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      return true;
    } else { return false; }
  } else { return false; }
}

bool SegFinderFilters::checkDistXY(std::string nameDistXY)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDistXY, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistXY();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      return true;
    } else { return false; }
  } else { return false; }
}

bool SegFinderFilters::checkDistZ(std::string nameDistZ)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDistZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      return true;
    } else { return false; }
  } else { return false; }
}

bool SegFinderFilters::checkNormedDist3D(std::string nameNormDist3D)    // max only
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameNormDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcNormedDist3D();
    if (aResult < pCutoff->getMaxValue()) {
      return true;
    } else { return false; }
  } else { return false; }
}

bool SegFinderFilters::checkSlopeRZ(std::string nameSlopeRZ)    // min & max
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameSlopeRZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcSlopeRZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      return true;
    } else { return false; }
  } else { return false; }
}


pair <double, double> SegFinderFilters::getCutoffs(string aFilter)
{
  pair <double, double> pair;
  Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) {
    pair = make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
    return pair;;
  } else { return make_pair(0., 0.); }
}
