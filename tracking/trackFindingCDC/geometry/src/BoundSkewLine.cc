/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/BoundSkewLine.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

/// Constuctor for a skew line between forward and backward point
BoundSkewLine::BoundSkewLine(const Vector3D& forwardIn,
                             const Vector3D& backwardIn) :
  m_refPos3D(backwardIn),
  m_movePerZ((forwardIn.xy() - backwardIn.xy()) / (forwardIn.z() - backwardIn.z())),
  m_forwardZ(forwardIn.z()),
  m_backwardZ(backwardIn.z())
{
  //skew and reference position are not a one liner to be filled to the init list
  //make some highschool math to get them

  Vector3D vectorAlongLine = forwardIn - backwardIn;

  FloatType weightForward  = forwardIn.dotXY(vectorAlongLine);
  FloatType weightBackward = -backwardIn.dotXY(vectorAlongLine);

  if (weightForward + weightBackward == 0) {
    //line is aligned along the z axes us the position in the xy plane as the reference
    //m_refPos3D was initialized as a copy of backwardIn so x, y are correct
    m_refPos3D.setZ(0.0);

  } else {
    vectorAlongLine *= (weightBackward / (weightForward + weightBackward));
    //m_refPos3D was initialized as a copy of backwardIn
    m_refPos3D += vectorAlongLine;
  }
}

BoundSkewLine BoundSkewLine::movedBy(const Vector3D& offset) const
{
  BoundSkewLine moved = *this;
  moved.m_refPos3D += offset.xy() + movePerZ() * offset.z();
  moved.m_forwardZ += offset.z();
  moved.m_backwardZ += offset.z();
  return moved;
}

BoundSkewLine BoundSkewLine::movedBy(const Vector2D& offset) const
{
  BoundSkewLine moved = *this;
  moved.m_refPos3D += offset;
  return moved;
}
