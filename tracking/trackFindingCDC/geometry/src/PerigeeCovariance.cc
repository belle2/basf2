/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/PerigeeCovariance.h>
#include <tracking/trackFindingCDC/geometry/EPerigeeParameter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void PerigeeCovariance::reverse()
{
  // Sign of impact and curvature are reversed by a reversion of the perigee parameters,
  // while the tangential phi0 gets only an offset by a constant.
  // Hence correlations between
  // * curvature and phi
  // * phi and impact
  // aquire a minus sign

  // receive a minus sign upon reversion.
  using namespace NPerigeeParameter;
  m_matrix(c_Curv, c_Phi0) = -m_matrix(c_Curv, c_Phi0);
  m_matrix(c_Phi0, c_Curv) = -m_matrix(c_Phi0, c_Curv);

  m_matrix(c_Phi0, c_I) = -m_matrix(c_Phi0, c_I);
  m_matrix(c_I, c_Phi0) = -m_matrix(c_I, c_Phi0);

}
