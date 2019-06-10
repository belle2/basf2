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

#include <analysis/variables/TriggerVariables.h>
#include <analysis/dataobjects/Particle.h>

// trigger dataobjects
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

// trigger dbobjects
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <mdst/dbobjects/TRGGDLDBPrescales.h>

// HLT dbobjects
#include <hlt/softwaretrigger/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>

#include <cmath>
#include <bitset> // bitwise stuff for L1

namespace Belle2 {
  namespace {
    /**
     * This function is the basis for returning in the variables for software trigger result.
     * It is hidden in a anonymous namespace.
     */
    double extractSoftwareTriggerResultImplementation(bool nonPrescaled, const std::string& triggerIdentifier, const Particle*)
    {
      // get trigger result object
      StoreObjPtr<SoftwareTriggerResult> swtr;
      if (!swtr) return std::numeric_limits<float>::quiet_NaN();

      // check that the trigger ID provided by the user exists in the SWTR
      SoftwareTriggerCutResult swtcr;
      try {
        if (nonPrescaled) {
          swtcr = swtr->getNonPrescaledResult(triggerIdentifier);
        } else {
          swtcr = swtr->getResult(triggerIdentifier);
        }
      } catch (const std::out_of_range&) {
        // then the trigger identifier is wrong
        std::string err = "The trigger identifier \"" + triggerIdentifier;
        err += "\" was not found. Maybe you misspelled it?\n";
        err += "Here are all possible trigger identifiers: \n";
        auto res = swtr->getResults();
        for (auto& re : res) err += re.first + "\n";
        B2FATAL(err);
      }
      return double(swtcr); // see mdst/dataobjects/include/SoftwareTriggerResult.h
    };
  }

  namespace Variable {

    double L1Trigger(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
      return trg->test();
    }

    Manager::FunctionPtr L1PSNM(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg) return std::numeric_limits<float>::quiet_NaN();
          return trg->testPsnm(name);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1PSNM function. The only argument must be the name of the PSNM trigger bit.");
      }
    }

    Manager::FunctionPtr L1FTDL(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg) return std::numeric_limits<float>::quiet_NaN();
          return trg->testFtdl(name);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1FTDL function. The only argument must be the name of the FTDL trigger bit.");
      }
    }

    Manager::FunctionPtr L1Input(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg) return std::numeric_limits<float>::quiet_NaN();
          return trg->testInput(name);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1Input function. The only argument must be the name of the input trigger bit.");
      }
    }

    Manager::FunctionPtr L1Prescale(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          static DBObjPtr<TRGGDLDBFTDLBits> ftdlBits;
          static DBObjPtr<TRGGDLDBPrescales> prescales;
          for (unsigned int bit = 0; bit < TRGSummary::c_trgWordSize * TRGSummary::c_ntrgWords; bit++)
          {
            if (std::string(ftdlBits->getoutbitname((int)bit)) == name) {
              return prescales->getprescales(bit);
            }
          }
          return std::numeric_limits<float>::quiet_NaN();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1Prescale function. The only argument must be the name of the PSNM trigger bit.");
      }
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
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
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
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
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
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
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
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
      prescale = trg->getPreScale(ntrgWord, bitInWord);

      return prescale;
    }

    double getTimType(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg) return std::numeric_limits<float>::quiet_NaN();
      return trg->getTimType();
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

      using namespace std::placeholders;
      return std::bind(extractSoftwareTriggerResultImplementation, false, triggerIdentifier, _1);
    }

    Manager::FunctionPtr softwareTriggerResultNonPrescaled(const std::vector<std::string>& args)
    {
      /* The analyst has to know the name of the trigger she wants
       * (after having looked this up from the trigger db payload)
       *
       * This workflow will probably improve later: but for now parse the args
       * to check we have one name, then check the name does not throw an
       * exception when we ask for it from the SWTR (std::map)
       */
      if (args.size() != 1)
        B2FATAL("Wrong number of arguments for the function softwareTriggerResultNonPrescaled");
      std::string triggerIdentifier = args[0];

      using namespace std::placeholders;
      return std::bind(extractSoftwareTriggerResultImplementation, true, triggerIdentifier, _1);
    }

    double passesAnyHighLevelTrigger(const Particle* p)
    {
      // for HLT, a c_accept is a pass and all other cases are fail
      // see mdst/dataobjects/include/SoftwareTriggerResult.h
      std::vector<std::string> hardcodedname
        = { "software_trigger_cut&filter&total_result" };
      double swtcr = softwareTriggerResult(hardcodedname)(p);
      if (swtcr > 0.5) return 1.0; // 1
      else             return 0.0; // 0 or -1
    }

    Manager::FunctionPtr softwareTriggerPrescaling(const std::vector<std::string>& args)
    {
      /* The analyst has to know the name of the trigger she wants
       * (after having looked this up from the trigger db payload)
       *
       * This workflow will probably improve later: but for now parse the args
       * to check we have one name, then check the name does not throw an
       * exception when we ask for it from the SWTR (std::map)
       */
      if (args.size() != 1)
        B2FATAL("Wrong number of arguments for the function softwareTriggerPrescaling");
      std::string triggerIdentifier = args[0];

      auto outputFunction = [triggerIdentifier](const Particle*) -> double {
        DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(triggerIdentifier);
        if (not downloadedCut)
        {
          B2FATAL("There is no trigger with the given name " << triggerIdentifier << "!");
        }

        return double(downloadedCut->getPreScaleFactor());
      };

      return outputFunction;
    }

    //-------------------------------------------------------------------------
    VARIABLE_GROUP("L1 Trigger");
    REGISTER_VARIABLE("L1Trigger", L1Trigger ,
                      "Returns 1 if at least one PSNM L1 trigger bit is true.");
    REGISTER_VARIABLE("L1PSNM(name)", L1PSNM ,
                      "Returns the PSNM (Prescale And Mask, after prescale) status of the trigger bit with the given name.");
    REGISTER_VARIABLE("L1FTDL(name)", L1FTDL ,
                      "Returns the FTDL (Final Trigger Decision Logic, before prescale) status of the trigger bit with the given name.");
    REGISTER_VARIABLE("L1Input(name)", L1Input,
                      "Returns the input bit status of the trigger bit with the given name.");
    REGISTER_VARIABLE("L1Prescale(name)", L1Prescale,
                      "Returns the PSNM (prescale and mask) prescale of the trigger bit with the given name.");
    REGISTER_VARIABLE("L1PSNMBit(i)", L1PSNMBit ,
                      "Returns the PSNM (Prescale And Mask, after prescale) status of i-th trigger bit.");
    REGISTER_VARIABLE("L1FTDLBit(i)", L1FTDLBit ,
                      "Returns the FTDL (Final Trigger Decision Logic, before prescale) status of i-th trigger bit.");
    REGISTER_VARIABLE("L1InputBit(i)", L1InputBit,
                      "Returns the input bit status of the i-th input trigger bit.");
    REGISTER_VARIABLE("L1PSNMBitPrescale(i)", L1PSNMBitPrescale,
                      "Returns the PSNM (prescale and mask) prescale of i-th trigger bit.");
    REGISTER_VARIABLE("L1TimType", getTimType ,
                      "Returns ETimingType time type.");
    //-------------------------------------------------------------------------
    VARIABLE_GROUP("Software Trigger");
    REGISTER_VARIABLE("SoftwareTriggerResult(triggerIdentifier)", softwareTriggerResult,
                      "[Eventbased] [Expert] returns the SoftwareTriggerCutResult, "
                      "defined as reject (-1), accept (1), or noResult (0). Note "
                      "that the meanings of these change depending if using trigger "
                      "or the skim stage, hence expert.");
    REGISTER_VARIABLE("SoftwareTriggerResultNonPrescaled(triggerIdentifier)", softwareTriggerResultNonPrescaled,
                      "[Eventbased] [Expert] returns the SoftwareTriggerCutResult, "
                      "if this trigger would not be prescaled."
                      "Please note, this is not the final HLT decision! "
                      "It is defined as reject (-1), accept (1), or noResult (0). Note "
                      "that the meanings of these change depending if using trigger "
                      "or the skim stage, hence expert.");
    REGISTER_VARIABLE("HighLevelTrigger", passesAnyHighLevelTrigger,
                      "[Eventbased] 1.0 if event passes the HLT trigger, 0.0 if not");
    REGISTER_VARIABLE("SoftwareTriggerPrescaling(triggerIdentifier)", softwareTriggerPrescaling,
                      "[Eventbased] return the prescaling for the specific software trigger identifier. "
                      "Please note, this prescaling is taken from the currently setup database. It only corresponds "
                      "to the correct HLT prescale if you are using the online database!");
    //-------------------------------------------------------------------------
  }
}
