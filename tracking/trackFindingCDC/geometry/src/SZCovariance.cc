/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/SZCovariance.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void SZCovariance::reverse()
{
  // Sign of slope (aka tan lambda) are reversed
  // while the z0 is unchanged.
  // Hence correlations between
  // * sz and z0
  // aquire a minus sign

  // receive a minus sign upon reversion.
  using namespace NHelixParameter;
  m_matrix(c_TanL, c_Z0) = -m_matrix(c_TanL, c_Z0);
  m_matrix(c_Z0, c_TanL) = -m_matrix(c_Z0, c_TanL);

}
