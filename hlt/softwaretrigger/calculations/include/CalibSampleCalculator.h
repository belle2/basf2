/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li                                               *
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
     * This calculator exports variables needed for the HLT part
     * of the path, e.g.
     * * AngGTHLT
     * * EC12CMSHLT
     * * etc.
     *
     * This class implements the two main functions requireStoreArrays and doCalculation of the
     * SoftwareTriggerCalculation class.
     */
    class CalibSampleCalculator : public SoftwareTriggerCalculation {
    public:
      /// Set the default names for the store object particle lists.
      CalibSampleCalculator() : m_pionParticles("pi+:calib"), m_gammaParticles("gamma:calib"), m_rhoParticles("rho0:calib"),
        m_D0Particles("D0:calib"), m_DstarParticles("D*+:calib"), m_XiParticles("Xi-:calib"), m_dqmD0("D0:dqm"), m_dqmDplus("D+:dqm"),
        m_dqmDstar("D*+:dqm"), m_dqmJpsiee("J/psi:dqm_ee"), m_dqmJpsimumu("J/psi:dqm_mumu") {}

      /// Require the particle list. We do not need more here.
      void requireStoreArrays() override;

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) const override;

    private:
      /// Internal storage of the tracks as particles.
      StoreObjPtr<ParticleList> m_pionParticles;
      /// Internal storage of the ECL clusters as particles.
      StoreObjPtr<ParticleList> m_gammaParticles;
      /// Intermediate state rho
      StoreObjPtr<ParticleList> m_rhoParticles;
      /// Intermediate state D0
      StoreObjPtr<ParticleList> m_D0Particles;
      /// Intermediate state Dstar
      StoreObjPtr<ParticleList> m_DstarParticles;
      /// Intermediate state Xi
      StoreObjPtr<ParticleList> m_XiParticles;
      ///D0 for dqm
      StoreObjPtr<ParticleList> m_dqmD0;
      ///D+ for dqm
      StoreObjPtr<ParticleList> m_dqmDplus;
      ///D*+ for dqm
      StoreObjPtr<ParticleList> m_dqmDstar;
      ///Jpsi -> ee for dqm
      StoreObjPtr<ParticleList> m_dqmJpsiee;
      ///Jpsi -> mumu for dqm
      StoreObjPtr<ParticleList> m_dqmJpsimumu;

    };
  }
}
