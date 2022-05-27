/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

using namespace Belle2;

ROOT::Math::PxPyPzMVector PCmsLabTransform::labToCms(const ROOT::Math::PxPyPzMVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

ROOT::Math::PxPyPzMVector PCmsLabTransform::cmsToLab(const ROOT::Math::PxPyPzMVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

ROOT::Math::PxPyPzEVector PCmsLabTransform::labToCms(const ROOT::Math::PxPyPzEVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

ROOT::Math::PxPyPzEVector PCmsLabTransform::cmsToLab(const ROOT::Math::PxPyPzEVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

PCmsLabTransform::PCmsLabTransform() = default;


