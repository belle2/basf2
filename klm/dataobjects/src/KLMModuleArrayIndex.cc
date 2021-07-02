/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMModuleArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMModuleArrayIndex::KLMModuleArrayIndex() :
  KLMElementArrayIndex(KLMChannelIndex::c_IndexLevelLayer)
{
}

KLMModuleArrayIndex::~KLMModuleArrayIndex()
{
}

const KLMModuleArrayIndex& KLMModuleArrayIndex::Instance()
{
  static KLMModuleArrayIndex klmModuleArrayIndex;
  return klmModuleArrayIndex;
}
