/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrajectorySZ.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TRACKFINDINGCDC_SwitchableClassImp(CDCTrajectorySZ)



CDCTrajectorySZ CDCTrajectorySZ::basicAssumption()
{
  // Dummy error estimates
  SZCovariance szCovariance;

  szCovariance(iSZ, iSZ) = 2.0; // Error in pz double the error in pt, good estimate?
  szCovariance(iZ0, iSZ) = 0.0;
  szCovariance(iSZ, iZ0) = 0.0;
  szCovariance(iZ0, iZ0) = 2.0;

  // A dummy line with no increasing z coordinate
  double tanLambda = 0.0;
  double z0 = 0.0;
  UncertainSZLine uncertainSZLine(tanLambda, z0, szCovariance);

  CDCTrajectorySZ trajectorySZ(uncertainSZLine);


  return trajectorySZ;
}
