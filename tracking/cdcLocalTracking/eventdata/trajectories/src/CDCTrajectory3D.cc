/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrajectory3D.h"
#include "../include/CDCBField.h"


#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCTrajectory3D)


CDCTrajectory3D::CDCTrajectory3D(const Vector3D& pos3D,
                                 const Vector3D& mom3D,
                                 const FloatType& charge) :
  m_localOrigin(pos3D),
  m_localHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D), mom3D.xy().unit(), 0.0, mom3D.cotTheta(), 0.0)
{
}



void CDCTrajectory3D::setPosMom3D(const Vector3D& pos3D,
                                  const Vector3D& mom3D,
                                  const FloatType& charge)
{
  m_localOrigin = pos3D;
  m_localHelix = UncertainHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D),
                                mom3D.xy().unit(),
                                0.0,
                                mom3D.cotTheta(),
                                0.0);
}



SignType CDCTrajectory3D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalHelix().circleXY().orientation());
}



FloatType CDCTrajectory3D::getAbsMom3D() const
{
  return curvatureToAbsMom2D(getLocalHelix().circleXY().curvature());
}




