/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/HelixCovariance.h>
#include <tracking/trackFindingCDC/geometry/EHelixParameter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void HelixCovariance::reverse()
{

  // Sign of impact, curvature and tanLambda are reversed by a reversion of the helix parameters,
  // while the tangential phi0 gets only an offset by a constant and z0 is unchanged
  // Hence correlations between
  // * curvature and phi
  // * curvature and z0
  // * phi and impact
  // * phi and tanLambda
  // * impact and z0
  // * tanLambda and z0
  // aquire a minus sign

  // receive a minus sign upon reversion.
  using namespace NHelixParameter;
  m_matrix(c_Curv, c_Phi0) = -m_matrix(c_Curv, c_Phi0);
  m_matrix(c_Phi0, c_Curv) = -m_matrix(c_Phi0, c_Curv);

  m_matrix(c_Curv, c_Z0) = -m_matrix(c_Curv, c_Z0);
  m_matrix(c_Z0, c_Curv) = -m_matrix(c_Z0, c_Curv);

  m_matrix(c_Phi0, c_I) = -m_matrix(c_Phi0, c_I);
  m_matrix(c_I, c_Phi0) = -m_matrix(c_I, c_Phi0);

  m_matrix(c_Phi0, c_TanL) = -m_matrix(c_Phi0, c_TanL);
  m_matrix(c_TanL, c_Phi0) = -m_matrix(c_TanL, c_Phi0);

  m_matrix(c_I, c_Z0) = -m_matrix(c_I, c_Z0);
  m_matrix(c_Z0, c_I) = -m_matrix(c_Z0, c_I);

  m_matrix(c_TanL, c_Z0) = -m_matrix(c_TanL, c_Z0);
  m_matrix(c_Z0, c_TanL) = -m_matrix(c_Z0, c_TanL);

}
