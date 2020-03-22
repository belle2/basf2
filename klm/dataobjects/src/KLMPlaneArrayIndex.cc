/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMPlaneArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMPlaneArrayIndex::KLMPlaneArrayIndex() :
  KLMElementArrayIndex(KLMChannelIndex::c_IndexLevelPlane)
{
}

KLMPlaneArrayIndex::~KLMPlaneArrayIndex()
{
}

const KLMPlaneArrayIndex& KLMPlaneArrayIndex::Instance()
{
  static KLMPlaneArrayIndex klmPlaneArrayIndex;
  return klmPlaneArrayIndex;
}
