/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "dqm/analysis/HistObject.h"

using namespace Belle2;

bool HistObject::update(TH1* hist)
{
  if (hist) {
    // usual check for nullptr
    m_updated |= hist->GetEntries() != m_entries;
    m_entries = hist->GetEntries();
  }
  m_hist = hist; // even if it is nullptr
  return m_updated;
}

void HistObject::resetBeforeEvent(void)
{
  m_hist = nullptr;
  m_updated = false;
}
