/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
