/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Francesco Tenchini                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

using namespace Belle2;

TLorentzVector PCmsLabTransform::labToCms(const TLorentzVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

TLorentzVector PCmsLabTransform::cmsToLab(const TLorentzVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

PCmsLabTransform::PCmsLabTransform() = default;

const BeamParameters& PCmsLabTransform::getBeamParams() const
{
  return *m_beamParamsDB;
}

