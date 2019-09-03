/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <framework/logging/Logger.h>
#include <klm/dataobjects/KLMModuleArrayIndex.h>

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
