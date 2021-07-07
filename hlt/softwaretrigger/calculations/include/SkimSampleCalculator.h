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
      /// Internal storage of the tracks as particles (definition for hadronb).
      StoreObjPtr<ParticleList> m_pionHadParticles;
      /// Internal storage of the tracks as particles (definition for tau skims).
      StoreObjPtr<ParticleList> m_pionTauParticles;
      /// Internal storage of the K_S0's.
      StoreObjPtr<ParticleList> m_KsParticles;
      /// Internal storage of the Lambda0's.
      StoreObjPtr<ParticleList> m_LambdaParticles;
      /// Internal storage of the D*'s.
      StoreObjPtr<ParticleList> m_DstParticles;
      /// Internal storage of the tracks for alignment calibration.
      StoreObjPtr<ParticleList> m_offIpParticles;
      /// HLT filter line for the TRG skim.
      std::string m_filterL1TrgNN = "";
    };
  }
}
