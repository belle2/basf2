/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


