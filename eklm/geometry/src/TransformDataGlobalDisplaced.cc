/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/geometry/TransformDataGlobalDisplaced.h>

using namespace Belle2;

const EKLM::TransformDataGlobalDisplaced&
EKLM::TransformDataGlobalDisplaced::Instance()
{
  static EKLM::TransformDataGlobalDisplaced t;
  return t;
}

EKLM::TransformDataGlobalDisplaced::TransformDataGlobalDisplaced() :
  EKLM::TransformData(true, true)
{
}

EKLM::TransformDataGlobalDisplaced::~TransformDataGlobalDisplaced()
{
}

