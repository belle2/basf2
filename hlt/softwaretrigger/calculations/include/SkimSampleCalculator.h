/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Chris Hearty, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <hlt/softwaretrigger/core/SoftwareTriggerObject.h>
#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * to fill a SoftwareTriggerObject for selecting particles for skimming and data quality monitoring.
     *
     * This class implements the two main functions requireStoreArrays and doCalculation of the
     * SoftwareTriggerCalculation class.
     */
    class SkimSampleCalculator : public SoftwareTriggerCalculation {
    public:
      /// Set the default names for the store object particle lists.
      SkimSampleCalculator();

      /// Require the particle list. We do not need more here.
      void requireStoreArrays() override;

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) override;

    private:
      /// Internal storage of the tracks as particles.
      StoreObjPtr<ParticleList> m_pionParticles;
      /// Internal storage of the ECL clusters as particles.
      StoreObjPtr<ParticleList> m_gammaParticles;
    };
  }
}
