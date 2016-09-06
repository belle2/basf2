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
      c_reject, /**< Reject this event. */
      c_accept, /**< Accept this event. */
      c_noResult /**< This cut did not give any information on what to do with the event. */
    };
  }
}
