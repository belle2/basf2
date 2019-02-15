/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Chris Hearty                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * to fill a SoftwareTriggerObject for doing HLT cuts.
     *
     * This calculator exports variables needed for the trigger HLT part
     * of the path.
     *
     * This class implements the two main functions requireStoreArrays and doCalculation of the
     * SoftwareTriggerCalculation class.
     */
    class FilterCalculator : public SoftwareTriggerCalculation {
    public:
      /// Set the default names for the store object particle lists.
      FilterCalculator() {}

      /// Require the particle list. We do not need more here.
      void requireStoreArrays() override;

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) override;
    };
  }
}