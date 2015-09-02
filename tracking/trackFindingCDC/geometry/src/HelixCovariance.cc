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
#include <tracking/trackFindingCDC/geometry/HelixParameterIndex.h>

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
  m_matrix(iCurv, iPhi0) = -m_matrix(iCurv, iPhi0);
  m_matrix(iPhi0, iCurv) = -m_matrix(iPhi0, iCurv);

  m_matrix(iCurv, iZ0) = -m_matrix(iCurv, iZ0);
  m_matrix(iZ0, iCurv) = -m_matrix(iZ0, iCurv);

  m_matrix(iPhi0, iI) = -m_matrix(iPhi0, iI);
  m_matrix(iI, iPhi0) = -m_matrix(iI, iPhi0);

  m_matrix(iPhi0, iTanL) = -m_matrix(iPhi0, iTanL);
  m_matrix(iTanL, iPhi0) = -m_matrix(iTanL, iPhi0);

  m_matrix(iI, iZ0) = -m_matrix(iI, iZ0);
  m_matrix(iZ0, iI) = -m_matrix(iZ0, iI);

  m_matrix(iTanL, iZ0) = -m_matrix(iTanL, iZ0);
  m_matrix(iZ0, iTanL) = -m_matrix(iZ0, iTanL);

}
