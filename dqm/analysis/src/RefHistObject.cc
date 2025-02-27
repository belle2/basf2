
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "dqm/analysis/RefHistObject.h"

using namespace Belle2;

void RefHistObject::makeReferenceCopy(void)
{
  auto orgref = m_refHist.get();
  if (m_refCopy != nullptr) {
    // if it exists already (->second call), we clear it only.
    // thus the pointer is not changed!
    m_refCopy->Reset();
    m_refCopy->Add(orgref);
  } else {
    // is orgref is nullptr, just make a copy
    if (orgref && (orgref->InheritsFrom("TH1C") or orgref->InheritsFrom("TH1S"))) {
      m_refCopy = std::unique_ptr<TH1> (new TH1F()); // we want it a float for better scaling
      orgref->Copy(*m_refCopy.get());
    } else if (orgref && (orgref->InheritsFrom("TH1I") or orgref->InheritsFrom("TH1L"))) {
      m_refCopy = std::unique_ptr<TH1> (new TH1D()); // we want it a float for better scaling
      orgref->Copy(*m_refCopy.get());
    } else {
      // keep TProfile, TH1F or TH1D
      m_refCopy = std::unique_ptr<TH1> ((TH1*)orgref->Clone());
    }
    m_refCopy->SetLineStyle(2);
    m_refCopy->SetLineColor(3);
    m_refCopy->SetFillColor(0);
    m_refCopy->SetStats(kFALSE);
  }
}

TH1* RefHistObject::getReference(void)
{
  makeReferenceCopy();
  return m_refCopy.get();
}
