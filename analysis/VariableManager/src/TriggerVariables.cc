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

    double L1InputBit(const Particle*, const std::vector<double>& bit)
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
      const unsigned int trgWord = trg->getInputBits(ntrgWord);
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

    Manager::FunctionPtr softwareTriggerResult(const std::vector<std::string>& args)
    {
      /* The analyst has to know the name of the trigger she wants
       * (after having looked this up from the trigger db payload)
       *
       * This workflow will probably improve later: but for now parse the args
       * to check we have one name, then check the name does not throw an
       * exception when we ask for it from the SWTR (std::map)
       */
      if (args.size() != 1)
        B2FATAL("Wrong number of arguments for the function softwareTriggerResult");
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
        return double(swtcr); // see mdst/dataobjects/include/SoftwareTriggerResult.h
      };
      return outputfunction;
    }

    double passesAnyHighLevelTrigger(const Particle* p)
    {
      // for HLT, a c_accept is a pass and all other cases are fail
      // see mdst/dataobjects/include/SoftwareTriggerResult.h
      std::vector<std::string> hardcodedname
        = { "software_trigger_cut&hlt&total_result" };
      double swtcr = softwareTriggerResult(hardcodedname)(p);
      if (swtcr > 0.5) return 1.0; // 1
      else             return 0.0; // 0 or -1
    }

    double passesAnyFastRecoTrigger(const Particle* p)
    {
      // for fast reco, a c_accept and c_noResult is a pass, c_reject is a fail
      // see mdst/dataobjects/include/SoftwareTriggerResult.h
      std::vector<std::string> hardcodedname
        = { "software_trigger_cut&fast_reco&total_result" };
      double swtcr = softwareTriggerResult(hardcodedname)(p);
      if (swtcr < -0.5) return 0.0; // -1
      else              return 1.0; // 1 or 0
    }

    //-------------------------------------------------------------------------
    VARIABLE_GROUP("L1 Trigger");
    REGISTER_VARIABLE("L1Trigger", L1Trigger ,
                      "Returns 1 if at least one PSNM L1 trigger bit is true.");
    REGISTER_VARIABLE("L1PSNMBit(i)", L1PSNMBit ,
                      "Returns the PSNM (Prescale And Mask, after prescale) status of i-th trigger bit.");
    REGISTER_VARIABLE("L1FTDLBit(i)", L1FTDLBit ,
                      "Returns the FTDL (Final Trigger Decision Logic, before prescale) status of i-th trigger bit.");
    REGISTER_VARIABLE("L1InputBit(i)", L1InputBit,
                      "Returns the input bit status of the i-th input trigger bit.");
    REGISTER_VARIABLE("L1PSNMBitPrescale(i)", L1PSNMBitPrescale,
                      "Returns the PSNM (prescale and mask) prescale of i-th trigger bit.");
    //-------------------------------------------------------------------------
    VARIABLE_GROUP("Software Trigger");
    REGISTER_VARIABLE("SoftwareTriggerResult(triggerIdentifier)", softwareTriggerResult,
                      "[Eventbased] [Expert] returns the SoftwareTriggerCutResult, "
                      "defined as reject (-1), accept (1), or noResult (0). Note "
                      "that the meanings of these change depending if using FastReco "
                      "or the HLT, hence expert.");
    REGISTER_VARIABLE("HighLevelTrigger", passesAnyHighLevelTrigger,
                      "[Eventbased] 1.0 if event passes the HLT trigger, 0.0 if not");
    REGISTER_VARIABLE("FastRecoTrigger", passesAnyFastRecoTrigger,
                      "[Eventbased] 1.0 if event passes the fastreco trigger, 0.0 if not");
    //-------------------------------------------------------------------------
  }
}
