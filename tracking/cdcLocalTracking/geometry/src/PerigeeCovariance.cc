/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PerigeeCovariance.h"
#include "../include/CovarianceMatrixIndices.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PerigeeCovariance)

void PerigeeCovariance::reverse()
{

  // Sign of impact and curvature are reversed by a reversion of the perigee parameters,
  // while the tangential phi0 gets only an offset by a constant.
  // Hence corraletions between
  // * curvature and phi
  // * phi and impact
  // aquire a minus sign

  // receive a minus sign upon reversion.
  m_matrix(iCurv, iPhi0) = -m_matrix(iCurv, iPhi0);
  m_matrix(iPhi0, iCurv) = -m_matrix(iPhi0, iCurv);

  m_matrix(iPhi0, iI) = -m_matrix(iPhi0, iI);
  m_matrix(iI, iPhi0) = -m_matrix(iI, iPhi0);


}
