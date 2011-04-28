/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentConstant.h>

using namespace std;
using namespace Belle2;


BFieldComponentConstant::BFieldComponentConstant()
{

}


BFieldComponentConstant::~BFieldComponentConstant()
{

}


TVector3 BFieldComponentConstant::calculate(const TVector3& point) const
{
  return TVector3(m_magneticField[0], m_magneticField[1], m_magneticField[2]);
}


void BFieldComponentConstant::setMagneticFieldValues(double x, double y, double z)
{
  m_magneticField[0] = x;
  m_magneticField[1] = y;
  m_magneticField[2] = z;
}
