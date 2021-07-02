/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

