/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/NbFinderFilters.h"
#include "../include/Cutoff.h"

using namespace std;
using namespace Belle2;


bool NbFinderFilters::checkAngle3D(std::string nameAngle3D)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameAngle3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcAngle3D();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_angle3DCtr.first++;
      return true;
    }
  }
  m_angle3DCtr.second++;
  return false;
}


bool NbFinderFilters::checkAngleXY(std::string nameAngleXY)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameAngleXY, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcAngleXY();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_angleXYCtr.first++;
      return true;
    }
  }
  m_angleXYCtr.second++;
  return false;
}


bool NbFinderFilters::checkAngleRZ(std::string nameAngleRZ)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameAngleRZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcAngleRZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_angleRZCtr.first++;
      return true;
    }
  }
  m_angleRZCtr.second++;
  return false;
}


bool NbFinderFilters::checkCircleDist2IP(std::string nameCircleDist2IP)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameCircleDist2IP, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcCircleDist2IP();
    if (aResult < pCutoff->getMaxValue()) {
      m_circleDist2IPCtr.first++;
      return true;
    }
  }
  m_circleDist2IPCtr.second++;
  return false;
}


bool NbFinderFilters::checkDeltaSlopeRZ(std::string nameDeltaSlopeRZ)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaSlopeRZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcDeltaSlopeRZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_deltaSlopeRZCtr.first++;
      return true;
    }
  }
  m_deltaSlopeRZCtr.second++;
  return false;
}


bool NbFinderFilters::checkPt(std::string namePt)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(namePt, m_friendID);
  if (pCutoff != NULL) {
    double aResult = ThreeHitFilters::calcPt();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_pTCtr.first++;
      return true;
    }
  }
  m_pTCtr.second++;
  return false;
}


bool NbFinderFilters::checkHelixFit(std::string nameHelixFit)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(nameHelixFit, m_friendID);
  if (pCutoff != NULL) {
    double aResult = this->calcHelixFit();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_helixFitCtr.first++;
      return true;
    }
  }
  m_helixFitCtr.second++;
  return false;
}


pair <double, double> NbFinderFilters::getCutoffs(string aFilter)
{
  Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) return make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
  return make_pair(0., 0.);
}
