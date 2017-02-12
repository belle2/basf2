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
  /**
   * Base class for the SoftwareTriggerCut and its DBRepresentation. This is needed as we do not want to upload the
   * GeneralCut with all of its pointers to the database, but rather decompile the cut back into a string before
   * and compile it again after down- and upload. This base class hosts all the properties that are common to both
   * implementations, which is the pre scale factor list and the cut type (reject or accept).
   *
   * Please refer to the child classes (especially to SoftwareTriggerCut) for mroe information.
   */
  class SoftwareTriggerCutBase {
  public:
    /// Create a new base instance. This should rarely be called by yourself.
    SoftwareTriggerCutBase(const std::vector<unsigned int>& preScaleFactor = {1}, const bool& isRejectCut = false) :
      m_preScaleFactor(preScaleFactor), m_isRejectCut(isRejectCut)
    {
    }

    /// Return the list of pre scale factors.
    const std::vector<unsigned int>& getPreScaleFactor() const
    {
      return m_preScaleFactor;
    }

    /// Returns true, if the cut is a reject cut and false otherwise.
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
