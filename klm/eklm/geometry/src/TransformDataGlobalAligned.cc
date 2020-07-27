/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/TransformDataGlobalAligned.h>

using namespace Belle2;

const EKLM::TransformDataGlobalAligned&
EKLM::TransformDataGlobalAligned::Instance()
{
  static EKLM::TransformDataGlobalAligned t;
  return t;
}

EKLM::TransformDataGlobalAligned::TransformDataGlobalAligned() :
  EKLM::TransformData(true, EKLM::TransformData::c_Alignment)
{
}

EKLM::TransformDataGlobalAligned::~TransformDataGlobalAligned()
{
}

