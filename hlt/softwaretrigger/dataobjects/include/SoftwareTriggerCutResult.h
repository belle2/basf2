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

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Enumeration with all possible results of the SoftwareTriggerCut.
    enum class SoftwareTriggerCutResult {
      c_reject = -1, /**< Reject this event. */
      c_accept = 1, /**< Accept this event. */
      c_noResult = 0 /**< There were not enough information to decide on what to do with the event. */
    };
  }
}
