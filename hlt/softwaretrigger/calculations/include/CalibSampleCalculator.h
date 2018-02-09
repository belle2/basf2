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
#include <string>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Implementation of a calculator used in the SoftwareTriggerModule
     * to fill a SoftwareTriggerObject for selecting particles for calibration and data quality monitoring.
     *
     * This class implements the two main functions requireStoreArrays and doCalculation of the
     * SoftwareTriggerCalculation class.
     */
    class CalibSampleCalculator : public SoftwareTriggerCalculation {
    public:
      /// Set the default names for the store object particle lists.
      CalibSampleCalculator(): m_particlelist(), m_extrainfoname() {};

      /// Set the default names for the store object particle lists.
      CalibSampleCalculator(std::vector<std::string> prt, std::vector<std::string>ext) { m_particlelist = prt; m_extrainfoname = ext;};

      /// Require the particle list. We do not need more here.
      void requireStoreArrays() override;

      /// Actually write out the variables into the map.
      void doCalculation(SoftwareTriggerObject& calculationResult) const override;


    private:
      /** required input for ParticleList */
      StoreObjPtr<ParticleList> m_particleList;

      /** the name of particle list */
      std::vector<std::string> m_particlelist;

      /** the name of extra info */
      std::vector<std::string> m_extrainfoname;
    };
  }
}
