/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/FourHitFilters.h"
#include <boost/math/special_functions/fpclassify.hpp>
#include <TMathBase.h>
#include <math.h>

using namespace std;
using namespace Belle2;

/// TODO: evtly do a 'nan'-check for return values


double FourHitFilters::deltapT()
{
  if (m_circleCalculated == false) { calcCircle(); }
  return abs(0.0045 * (m_radiusABC - m_radiusBCD)); // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.0045*r[cm]
}



double FourHitFilters::deltaDistCircleCenter()   // instead of comparing both distance2IP-values, the real circleCenters will be compared
{
  if (m_circleCalculated == false) { calcCircle(); }
  return (abs(m_centerABC.Perp() - m_centerBCD.Perp())); // distance between both guesses of the intersection point
} // return unit: cm



void FourHitFilters::calcCircle()
{
  m_threeHitFilter.calcCircleCenter(m_hitA, m_hitB, m_hitC, m_centerABC);
  m_radiusABC = m_threeHitFilter.calcRadius(m_hitA, m_hitB, m_hitC, m_centerABC);
  m_threeHitFilter.calcCircleCenter(m_hitB, m_hitC, m_hitD, m_centerBCD);
  m_radiusBCD = m_threeHitFilter.calcRadius(m_hitB, m_hitC, m_hitD, m_centerBCD);

  m_circleCalculated = true;
}
