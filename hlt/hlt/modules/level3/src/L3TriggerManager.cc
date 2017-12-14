/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include "hlt/hlt/modules/level3/L3TriggerManager.h"

#include "hlt/hlt/modules/level3/L3TrackTrigger.h"
#include "hlt/hlt/modules/level3/L3EnergyTrigger.h"

using namespace Belle2;

void L3TriggerManager::initializeTriggerManager()
{
  m_hltTag.registerInDataStore();
  if (m_saveData) m_l3Tag.registerInDataStore();
}

void L3TriggerManager::printInformation()
{
  std::stringstream ss;
  const int n = m_triggers.length();
  for (int i = 0; i < n; i++) {
    if (i) ss << ", ";
    ss << m_triggers[i]->GetName() << "(=bit" << i + 1 << ")";
  }
  B2INFO("Level3: Trigger algorithm: " << ss.str());
}

bool L3TriggerManager::preselect()
{
  StoreArray<HLTTag> hltTags;
  if (!hltTags.getEntries()) hltTags.appendNew(HLTTag());
  HLTTag& hltTag = *hltTags[0];
  int summary(0);
  int bitMask(0x2);
  const int n = m_triggers.length();
  for (int i = 0; i < n; i++) {
    if (m_triggers[i]->preselect()) summary |= (bitMask | 0x1);
    bitMask <<= 1;
  }
  if (summary & 0x1) hltTag.Accept(HLTTag::Level3);
  hltTag.SetAlgoInfo(HLTTag::Level3, summary);
  if (m_saveData) {
    StoreArray<L3Tag> l3Tags;
    l3Tags.clear();
    l3Tags.appendNew(L3Tag(summary));
  }
  return summary & 0x1;
}

bool L3TriggerManager::select()
{
  StoreArray<HLTTag> hltTags;
  HLTTag& hltTag = *hltTags[0];
  L3Tag* l3Tag = NULL;
  if (m_saveData) {
    StoreArray<L3Tag> l3Tags;
    l3Tag = l3Tags[0];
  }
  int summary = hltTag.GetAlgoInfo(HLTTag::Level3);
  int bitMask(0x2);
  const int n = m_triggers.length();
  for (int i = 0; i < n; i++) {
    if (m_triggers[i]->select(l3Tag)) summary |= (bitMask | 0x1);
    bitMask <<= 1;
  }
  if (summary & 0x1) hltTag.Accept(HLTTag::Level3);
  else hltTag.Discard(HLTTag::Level3);
  hltTag.SetAlgoInfo(HLTTag::Level3, summary);
  if (m_saveData) l3Tag->setSummaryWord(summary);
  return summary & 0x1;
}
