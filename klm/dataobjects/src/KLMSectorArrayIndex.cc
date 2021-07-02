/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMSectorArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMSectorArrayIndex::KLMSectorArrayIndex() :
  KLMElementArrayIndex(KLMChannelIndex::c_IndexLevelSector)
{
}

KLMSectorArrayIndex::~KLMSectorArrayIndex()
{
}

const KLMSectorArrayIndex& KLMSectorArrayIndex::Instance()
{
  static KLMSectorArrayIndex klmSectorArrayIndex;
  return klmSectorArrayIndex;
}
