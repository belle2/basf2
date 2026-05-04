/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerObject.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <hlt/utilities/HLTPrefilter.h>

namespace Belle2::SoftwareTrigger {
  /**
   * Implementation of a calculator used in the SoftwareTriggerModule
   * to fill a SoftwareTriggerObject for doing HLT prefilter cuts.
   *
   * This calculator exports variables needed for the trigger HLT prefilter part
   * of the path ( = filtering out injection background events)
   *
   * This class implements the two main functions requireStoreArrays and doCalculation of the
   * SoftwareTriggerCalculation class.
   */
  class PrefilterCalculator : public SoftwareTriggerCalculation {
  public:
    /// Set the default names for the store object particle lists.
    PrefilterCalculator();

    /// Require the particle list. We do not need more here.
    void requireStoreArrays() override;

    /// Actually write out the variables into the map.
    void doCalculation(SoftwareTriggerObject& calculationResult) override;

  private:
    /// Helper instance for EventsOfDoom prefilter
    HLTPrefilter::EventsOfDoomBusterState m_eodbPrefilter;
    /// Helper instance for timing based prefilter
    HLTPrefilter::TimingCutState m_timingPrefilter;

  };
}
