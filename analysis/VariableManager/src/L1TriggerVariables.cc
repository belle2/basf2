/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/L1TriggerVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/TRGSummary.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <cmath>

#include <bitset>

namespace Belle2 {
  namespace Variable {

    double L1Trigger(const Particle* part)
    {
      const unsigned int ntrgWords = 10;

      StoreObjPtr<TRGSummary> trg;
      for (unsigned int i = 0; i < ntrgWords; ++i) {
        if (trg->getTRGSummary(i) > 0) return 1.0;
      }

      return 0.0;
    }

    double L1TriggerBit(const Particle* part, const std::vector<double>& bit)
    {
      double isL1Trigger = 0.0;

      if (bit.size() != 1) return isL1Trigger;

      // The number of trigger words is hardcoded in the mdst dataobject and no getter for the full array exists
      const unsigned int trgWordSize = 32;
      const unsigned int ntrgWords = 10;
      if (bit[0] >= trgWordSize * ntrgWords or bit[0] < 0)  return isL1Trigger;

      // Get the trigger word that contains this bit (we could also convert the full array into a bitset or vector<bool> but that is a bit slower)
      const unsigned int ntrgWord = (int) bit[0] / trgWordSize;

      // Get the bit by right shifting the desired bit into the least significant position and masking it with 1.
      StoreObjPtr<TRGSummary> trg;
      const unsigned int trgWord = trg->getTRGSummary(ntrgWord);
      const unsigned int bitInWord = ((unsigned int) bit[0] - ntrgWord * trgWordSize);
      isL1Trigger = (trgWord >> bitInWord) & 1;

      return isL1Trigger;
    }


    double L1TriggerBitPrescale(const Particle* part, const std::vector<double>& bit)
    {
      double prescale = 0.0;

      if (bit.size() != 1) return prescale;

      // The number of trigger words is hardcoded in the mdst dataobject
      const unsigned int trgWordSize = 32;
      const unsigned int ntrgWords = 10;
      if (bit[0] >= trgWordSize * ntrgWords or bit[0] < 0)  return prescale;

      // Get the prescale word that contains this bit
      const unsigned int ntrgWord = (int) bit[0] / trgWordSize;
      const unsigned int bitInWord = ((unsigned int) bit[0] - ntrgWord * trgWordSize);

      StoreObjPtr<TRGSummary> trg;
      prescale = trg->getPreScale(ntrgWord, bitInWord);

      return prescale;
    }


    VARIABLE_GROUP("L1Trigger");
    REGISTER_VARIABLE("L1Trigger", L1Trigger ,
                      "Returns 1 if at least one L1 trigger bit is true.");
    REGISTER_VARIABLE("L1TriggerBit(i)", L1TriggerBit ,
                      "Returns the status of i-th trigger bit.");
    REGISTER_VARIABLE("L1TriggerBitPrescale(i)", L1TriggerBitPrescale,
                      "Returns the prescale of i-th trigger bit.");

  }
}
