/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "dqm/analysis/RefHistObject.h"

using namespace Belle2;

RefHistObject::~RefHistObject()
{
}

bool RefHistObject::update(TH1* ref, double norm)
{
  // usual check for nullptr
  m_updated |= norm != m_refCopy->Integral();
  m_refHist = std::unique_ptr<TH1>(ref);
  if (!ref) {
    if (ref->InheritsFrom("TH1C") or ref->InheritsFrom("TH1S")) {
      m_refCopy = new TH1F(); // we want it a float for better scaling
      m_refHist->Copy(*m_refCopy);
    } else if (ref->InheritsFrom("TH1I") or ref->InheritsFrom("TH1L")) {
      m_refCopy = new TH1D(); // we want it a float for better scaling
      m_refHist->Copy(*m_refCopy);
    } else {
      // keep TProfile, TH1F or TH1D
      m_refCopy = (TH1*)m_refHist->Clone();
    }
  }
  if (!m_refCopy)
    m_refCopy = std::unique_ptr<TH1>(m_refCopy->Scale(norm / m_refCopy->Integral()););

  return m_updated; //incase an update procedure is needed
}

void RefHistObject::resetBeforeEvent(void)
{
  m_refHist = nullptr;
  m_refCopy = nullptr;
  m_canvas = nullptr;
  //m_updated = false; //incase an update procedure is needed
}
