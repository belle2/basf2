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
