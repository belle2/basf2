/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/geometry/SZParameters.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SZCovariance CDCTrajectorySZ::getBasicSZCovariance()
{
  // Dummy error estimates
  SZCovariance szCovariance;
  using namespace NSZParameterIndices;
  szCovariance(c_TanL, c_TanL) = 2.0; // Error in pz double the error in pt, good estimate?
  szCovariance(c_Z0, c_TanL) = 0.0;
  szCovariance(c_TanL, c_Z0) = 0.0;
  szCovariance(c_Z0, c_Z0) = 2.0;
  return szCovariance;
}

CDCTrajectorySZ CDCTrajectorySZ::basicAssumption()
{
  // A dummy line with no increasing z coordinate
  double tanLambda = 0.0;
  double z0 = 0.0;
  SZCovariance szCovariance = getBasicSZCovariance();
  UncertainSZLine uncertainSZLine(tanLambda, z0, szCovariance);
  CDCTrajectorySZ trajectorySZ(uncertainSZLine);
  return trajectorySZ;
}
