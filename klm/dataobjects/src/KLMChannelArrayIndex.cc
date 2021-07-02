/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMChannelArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

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
