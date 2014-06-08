/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/NbFinderFilters.h"
#include "tracking/vxdCaTracking/Cutoff.h"
#include <TRandom.h>

using namespace std;
using namespace Belle2;


bool NbFinderFilters::checkAngle3D(int nameAngle3D)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameAngle3D, m_friendID);
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


bool NbFinderFilters::checkAngleXY(int nameAngleXY)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameAngleXY, m_friendID);
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


bool NbFinderFilters::checkAngleRZ(int nameAngleRZ)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameAngleRZ, m_friendID);
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


bool NbFinderFilters::checkCircleDist2IP(int nameCircleDist2IP)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameCircleDist2IP, m_friendID);
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


bool NbFinderFilters::checkDeltaSlopeRZ(int nameDeltaSlopeRZ)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaSlopeRZ, m_friendID);
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


bool NbFinderFilters::checkPt(int namePt)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(namePt, m_friendID);
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


bool NbFinderFilters::checkHelixParameterFit(int nameHelixParameterFit)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameHelixParameterFit, m_friendID);
  if (pCutoff != NULL) {
    double aResult = this->calcHelixFit();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_helixParameterFitCtr.first++;
      return true;
    }
  }
  m_helixParameterFitCtr.second++;
  return false;
}

bool NbFinderFilters::checkDeltaSOverZ(int nameDeltaSOverZ)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaSOverZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = this->calcDeltaSOverZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_deltaSOverZCtr.first++;
      return true;
    }
  }
  m_deltaSOverZCtr.second++;
  return false;
}

bool NbFinderFilters::checkDeltaSlopeZOverS(int nameDeltaSlopeZOverS)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDeltaSlopeZOverS, m_friendID);
  if (pCutoff != NULL) {
    double aResult = this->calcDeltaSlopeZOverS();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_deltaSlopeZOverSCtr.first++;
      return true;
    }
  }
  m_deltaSlopeZOverSCtr.second++;
  return false;
}

bool NbFinderFilters::checkAlwaysTrue3Hit(int nameAlwaysTrue3Hit)
{
  if (std::isnan(nameAlwaysTrue3Hit) == true) { return false; }
  return true;
}

bool NbFinderFilters::checkAlwaysFalse3Hit(int nameAlwaysFalse3Hit)
{
  if (std::isnan(nameAlwaysFalse3Hit) == true) { return true; }
  return false;
}


bool NbFinderFilters::checkRandom3Hit(int nameRandom3Hit)
{
  if (std::isnan(nameRandom3Hit) == true) { return false; }
  double randomVal = gRandom->Uniform(0, 1);
  if (randomVal > 0.5) { return true; } // 50% chance of getting true
  return false;
}


double NbFinderFilters::calcRandom3Hit() { return gRandom->Uniform(0, 1); }