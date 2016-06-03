/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/LineCovariance.h>
#include <tracking/trackFindingCDC/geometry/ELineParameter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void LineCovariance::reverse()
{
  // The sign of impact is reversed by a reversion of the line parameters,
  // while the tangential phi0 gets only an offset by a constant and z0 is unchanged
  // Hence correlations between
  // * phi and impact
  // aquire a minus sign

  // receive a minus sign upon reversion.
  using namespace NLineParameter;
  m_matrix(c_Phi0, c_I) = -m_matrix(c_Phi0, c_I);
  m_matrix(c_I, c_Phi0) = -m_matrix(c_I, c_Phi0);
}
