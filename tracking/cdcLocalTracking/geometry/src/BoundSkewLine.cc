/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/BoundSkewLine.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(BoundSkewLine)

/// Constuctor for a skew line between forward and backward point
BoundSkewLine::BoundSkewLine(
  const Vector3D& forwardIn,
  const Vector3D& backwardIn
) :
  m_forwardZ(forwardIn.z()),
  m_backwardZ(backwardIn.z()),
  m_skew(0.0),
  m_referencePosition(backwardIn)
{

  //skew and reference position are not a one liner to be filled to the init list
  //make some highschool math to get them

  Vector3D vectorAlongLine = forwardIn - backwardIn;

  FloatType weight_forward   =  forwardIn.dotXY(vectorAlongLine);
  FloatType weight_backward  = -backwardIn.dotXY(vectorAlongLine);

  if (weight_forward + weight_backward == 0) {
    //line is aligned along the z axes us the position in the xy plane as the reference

    m_skew = 0.0;
    //m_referencePosition was initialized as a copy of backwardIn so x, y are correct
    m_referencePosition.setZ(0.0);

  } else {

    vectorAlongLine *= (weight_backward / (weight_forward + weight_backward));
    //m_referencePosition was initialized as a copy of backwardIn
    m_referencePosition += vectorAlongLine;

    m_skew = (m_referencePosition.crossXY(vectorAlongLine.xy())) /
             /*---------------------------------------------------------*/
             (m_referencePosition.polarRSquared() * vectorAlongLine.z());

  }
}
