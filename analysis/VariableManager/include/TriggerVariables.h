/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber, Sam Cunliffe                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <analysis/VariableManager/Manager.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns true if any L1 PSNM bit is true
     */
    double L1Trigger(const Particle*);

    /**
     * returns trigger FTDL bit (Final Trigger Decision Logic before prescale)
     */
    double L1FTDLBit(const Particle*, const std::vector<double>& bit);

    /**
     * returns trigger PSNM bit (prescale and mask), i.e. after prescale
     */
    double L1PSNMBit(const Particle*, const std::vector<double>& bit);

    /**
     * returns trigger prescale for a given trigger bit
     */
    double L1PSNMBitPrescale(const Particle*, const std::vector<double>& bit);

    /**
     * returns 1 if the event passes a given software trigger identifier
     */
    Manager::FunctionPtr passesSoftwareTrigger(const std::vector<std::string>& args);

    /**
     * returns 1 if the event passes the software trigger
     */
    double passesAnySoftwareTrigger(const Particle*);
  }
}
