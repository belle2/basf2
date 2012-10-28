/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/TrackOld.h>

#include <cmath>
#include <cstdlib>

using namespace std;
using namespace Belle2;

ClassImp(TrackOld)

TVector3 TrackOld::getMomentum() const
{
  TVector3 momentum(0., 0., 0.);
  double alpha = 1 / (1.5 * 0.00299792458); //assume constant 1.5 T magnetic field

  momentum.SetX(abs(1 / (m_omega * alpha))*cos(m_phi));
  momentum.SetY(abs(1 / (m_omega * alpha))*sin(m_phi));
  momentum.SetZ(abs(1 / (m_omega * alpha))*m_cotTheta);

  return momentum;

}

