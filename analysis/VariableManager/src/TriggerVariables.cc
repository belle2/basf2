/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *               Sam Cunliffe  (sam.cunliffe@desy.de)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/TriggerVariables.h>
#include <analysis/dataobjects/Particle.h>

// trigger dataobjects
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <cmath>
#include <bitset> // bitwise stuff for L1

namespace Belle2 {
  namespace Variable {

    double L1Trigger(const Particle*)
    {
      const unsigned int ntrgWords = 10;

      StoreObjPtr<TRGSummary> trg;
      for (unsigned int i = 0; i < ntrgWords; ++i) {
        if (trg->getPsnmBits(i) > 0) return 1.0;
      }

      return 0.0;
    }

    double L1PSNMBit(const Particle*, const std::vector<double>& bit)
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
      const unsigned int trgWord = trg->getPsnmBits(ntrgWord);
      const unsigned int bitInWord = ((unsigned int) bit[0] - ntrgWord * trgWordSize);
      isL1Trigger = (trgWord >> bitInWord) & 1;

      return isL1Trigger;
    }

    double L1FTDLBit(const Particle*, const std::vector<double>& bit)
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
      const unsigned int trgWord = trg->getFtdlBits(ntrgWord);
      const unsigned int bitInWord = ((unsigned int) bit[0] - ntrgWord * trgWordSize);
      isL1Trigger = (trgWord >> bitInWord) & 1;

      return isL1Trigger;
    }

    double L1PSNMBitPrescale(const Particle*, const std::vector<double>& bit)
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

    Manager::FunctionPtr passesSoftwareTrigger(const std::vector<std::string>& args)
    {
      /* The analyst has to know the name of the trigger she wants
       * (after having looked this up from the trigger db payload)
       *
       * This workflow will probably improve later: but for now parse the args
       * to check we have one name, then check the name does not throw an
       * exception when we ask for it from the SWTR (std::map)
       */
      if (args.size() != 1)
        B2FATAL("Wrong number of arguments for the function hltPass");
      std::string triggerIdentifier = args[0];

      // need to output a function for the VariableManager
      auto outputfunction = [triggerIdentifier](const Particle*) -> double {

        // get trigger result object
        StoreObjPtr<SoftwareTriggerResult> swtr;
        if (!swtr) return std::numeric_limits<float>::quiet_NaN();

        // check that the trigger ID provided by the user exists in the SWTR
        SoftwareTriggerCutResult swtcr;
        try {
          swtcr = swtr->getResult(triggerIdentifier);
        } catch (std::out_of_range)
        {
          // then the trigger indentifier is wrong
          std::string err = "The trigger identifier \"" + triggerIdentifier;
          err += "\" was not found. Maybe you misspelled it?\n";
          err += "Here are all possible trigger identifiers: \n";
          auto res = swtr->getResults();
          for (auto it = res.begin(); it != res.end(); it++) err += it->first + "\n";
          B2FATAL(err);
        }

        // now "unpack" and return the result
        switch (swtcr)
        {
          case SoftwareTriggerCutResult::c_reject:   return 0.0;
          case SoftwareTriggerCutResult::c_accept:   return 1.0;
          case SoftwareTriggerCutResult::c_noResult: return -1.0;
          default:
            B2ERROR("Trigger identifier \"" << triggerIdentifier << "\" has no result.");
            return std::numeric_limits<float>::quiet_NaN();
        };
      };
      return outputfunction;
    }

    double passesAnySoftwareTrigger(const Particle* p)
    {
      std::vector<std::string> hardcodedname
        = { "software_trigger_cut&hlt&total_result" };
      return passesSoftwareTrigger(hardcodedname)(p);
    }

    //-------------------------------------------------------------------------
    VARIABLE_GROUP("L1 Trigger");
    REGISTER_VARIABLE("L1Trigger", L1Trigger ,
                      "Returns 1 if at least one PSNM L1 trigger bit is true.");
    REGISTER_VARIABLE("L1PSNMBit(i)", L1PSNMBit ,
                      "Returns the PSNM status of i-th trigger bit.");
    REGISTER_VARIABLE("L1FTDLBit(i)", L1FTDLBit ,
                      "Returns the FTDL (Final Trigger Decision Logic before prescale) status of i-th trigger bit.");
    REGISTER_VARIABLE("L1PSNMBitPrescale(i)", L1PSNMBitPrescale,
                      "Returns the PSNM (prescale and mask) prescale of i-th trigger bit.");
    //-------------------------------------------------------------------------
    VARIABLE_GROUP("Software Trigger");
    REGISTER_VARIABLE("SoftwareTrigger(triggerIdentifier)", passesSoftwareTrigger,
                      "[Eventbased] 1.0 if event passes a given triggerID, 0.0 if it was rejected, -1.0 if no decision could be made");
    REGISTER_VARIABLE("SoftwareTrigger", passesAnySoftwareTrigger,
                      "[Eventbased] 1.0 if event passes the SW trigger, 0.0 if it was rejected, -1.0 if no decision could be made");
    //-------------------------------------------------------------------------
  }
}
