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
#include <klm/dataobjects/KLMChannelArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMChannelArrayIndex::KLMChannelArrayIndex() :
  KLMElementArrayIndex(KLMChannelIndex::c_IndexLevelStrip)
{
}

KLMChannelArrayIndex::~KLMChannelArrayIndex()
{
}

const KLMChannelArrayIndex& KLMChannelArrayIndex::Instance()
{
  static KLMChannelArrayIndex klmChannelArrayIndex;
  return klmChannelArrayIndex;
}
