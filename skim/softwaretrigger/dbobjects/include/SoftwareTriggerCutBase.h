/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <vector>

namespace Belle2 {
  class SoftwareTriggerCutBase {
  public:
    SoftwareTriggerCutBase(const std::vector<unsigned int>& preScaleFactor = {1}, const bool& isRejectCut = false) :
      m_preScaleFactor(preScaleFactor), m_isRejectCut(isRejectCut)
    {
    }

    const std::vector<unsigned int>& getPreScaleFactor() const
    {
      return m_preScaleFactor;
    }

    bool isRejectCut() const
    {
      return m_isRejectCut;
    }

  private:
    /// Do not copy/assign this object.
    SoftwareTriggerCutBase& operator=(const SoftwareTriggerCutBase&) = delete;
    /// Do not copy this object.
    SoftwareTriggerCutBase(const SoftwareTriggerCutBase& rhs) = delete;

    /// The internal storage of the prescale factor of the cut.
    std::vector<unsigned int> m_preScaleFactor = {1};
    /// The internal storage if it is a reject cut.
    bool m_isRejectCut = false;
  };
}
