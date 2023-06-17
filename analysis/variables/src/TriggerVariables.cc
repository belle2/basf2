/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/TriggerVariables.h>

#include <analysis/dataobjects/Particle.h>

// trigger dataobjects
#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

// trigger dbobjects
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <mdst/dbobjects/TRGGDLDBPrescales.h>

// HLT dbobjects
#include <mdst/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/Conversion.h>

// boost
#include <boost/algorithm/string.hpp>

// C++
#include <limits>
#include <stdexcept>

namespace Belle2 {
  namespace {
    /**
     * There are two ways to write a trigger identifier, either:
     *
     *    "software_trigger_cut&base_identifier&cut_identifier"
     *
     * or the format provided by the CLI `hlt_triggers print --list` :
     *
     *    "base_identifier cut_identifier" // with a space
     *
     * In case the latter is provided by the user, then convert to the former.
     * In case of the former, do nothing. Return the input (idempotent function).
     *
     * This function is hidden in a anonymous namespace -- only needed in this file.
     */
    std::string fullFormatIdentifier(const std::string& identifier)
    {
      std::string out = identifier;
      boost::replace_all(out, " ", "&");
      if (identifier.substr(0, 21) != "software_trigger_cut&")
        out = "software_trigger_cut&" + out;
      return out;
    }

    /**
     * This function is the basis for returning in the variables for software trigger result.
     * Also hidden in a anonymous namespace.
     */
    double extractSoftwareTriggerResultImplementation(bool nonPrescaled, const std::string& triggerIdentifier, const Particle*)
    {
      // get trigger result object
      StoreObjPtr<SoftwareTriggerResult> swtr;
      if (!swtr)
        return Const::doubleNaN;

      // check that the trigger ID provided by the user exists in the SWTR
      SoftwareTriggerCutResult swtcr;
      try {
        if (nonPrescaled) {
          swtcr = swtr->getNonPrescaledResult(fullFormatIdentifier(triggerIdentifier));
        } else {
          swtcr = swtr->getResult(fullFormatIdentifier(triggerIdentifier));
        }
      } catch (const std::out_of_range&) {
        // then the trigger identifier is wrong -- silently return nan
        return Const::doubleNaN;
      }
      return double(swtcr); // see mdst/dataobjects/include/SoftwareTriggerResult.h
    };
  }

  namespace Variable {

    double L1Trigger(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg)
        return Const::doubleNaN;
      return trg->test();
    }

    Manager::FunctionPtr L1PSNM(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg)
            return Const::doubleNaN;
          try
          {
            return trg->testPsnm(name);
          } catch (const std::exception&)
          {
            // Something went wrong, return NaN.
            return Const::doubleNaN;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1PSNM function. The only argument must be the name of the PSNM trigger bit.");
      }
    }

    double L1PSNMBit(const Particle*, const std::vector<double>& arguments)
    {
      if (arguments.size() == 1) {
        int testBit = std::lround(arguments[0]);

        StoreObjPtr<TRGSummary> trg;
        if (!trg)
          return Const::doubleNaN;
        try {
          return trg->testPsnm(testBit);
        } catch (const std::exception&) {
          // Something went wrong, return NaN.
          return Const::doubleNaN;
        }
      } else {
        B2FATAL("Wrong number of arguments for L1PSNMBit function. The only argument must be the number of the PSNM trigger bit.");
      }
    }

    Manager::FunctionPtr L1FTDL(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg)
            return Const::doubleNaN;
          try
          {
            return trg->testFtdl(name);
          } catch (const std::exception&)
          {
            // Something went wrong, return NaN.
            return Const::doubleNaN;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1FTDL function. The only argument must be the name of the FTDL trigger bit.");
      }
    }

    double L1FTDLBit(const Particle*, const std::vector<double>& arguments)
    {
      if (arguments.size() == 1) {
        int testBit = std::lround(arguments[0]);

        StoreObjPtr<TRGSummary> trg;
        if (!trg)
          return Const::doubleNaN;
        try {
          return trg->testFtdl(testBit);
        } catch (const std::exception&) {
          // Something went wrong, return NaN.
          return Const::doubleNaN;
        }
      } else {
        B2FATAL("Wrong number of arguments for L1FTDLBit function. The only argument must be the number of the FTDL trigger bit.");
      }
    }

    Manager::FunctionPtr L1Input(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          StoreObjPtr<TRGSummary> trg;
          if (!trg)
            return Const::doubleNaN;
          try
          {
            return trg->testInput(name);
          } catch (const std::exception&)
          {
            // Something went wrong, return NaN.
            return Const::doubleNaN;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1Input function. The only argument must be the name of the input trigger bit.");
      }
    }

    double L1InputBit(const Particle*, const std::vector<double>& arguments)
    {
      if (arguments.size() == 1) {
        int testBit = std::lround(arguments[0]);

        StoreObjPtr<TRGSummary> trg;
        if (!trg)
          return Const::doubleNaN;
        try {
          return trg->testInput(testBit);
        } catch (const std::exception&) {
          // Something went wrong, return NaN.
          return Const::doubleNaN;
        }
      } else {
        B2FATAL("Wrong number of arguments for L1Input function. The only argument must be the number of the input trigger bit.");
      }
    }

    Manager::FunctionPtr L1PSNMPrescale(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto name = arguments[0];
        auto func = [name](const Particle*) -> double {
          static DBObjPtr<TRGGDLDBFTDLBits> ftdlBits;
          if (!ftdlBits.isValid())
            return Const::doubleNaN;
          static DBObjPtr<TRGGDLDBPrescales> prescales;
          if (!prescales.isValid())
            return Const::doubleNaN;
          for (unsigned int bit = 0; bit < TRGSummary::c_trgWordSize * TRGSummary::c_ntrgWords; bit++)
          {
            if (std::string(ftdlBits->getoutbitname((int)bit)) == name)
              return prescales->getprescales(bit);
          }
          return Const::doubleNaN;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for L1Prescale function. The only argument must be the name of the PSNM trigger bit.");
      }
    }

    double L1PSNMBitPrescale(const Particle*, const std::vector<double>& arguments)
    {
      if (arguments.size() == 1) {
        int testBit = std::lround(arguments[0]);

        if (testBit < 0 or testBit >= (int)TRGSummary::c_trgWordSize * (int)TRGSummary::c_ntrgWords)
          return Const::doubleNaN;
        static DBObjPtr<TRGGDLDBPrescales> prescales;
        if (!prescales.isValid())
          return Const::doubleNaN;
        return prescales->getprescales(testBit);
      } else {
        B2FATAL("Wrong number of arguments for L1BitPrescale function. The only argument must be the number of the PSNM trigger bit.");
      }
    }

    double L1TimeType(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg)
        return Const::doubleNaN;
      return trg->getTimType();
    }

    double L1TimeQuality(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg)
        return Const::doubleNaN;
      return trg->getTimQuality();
    }

    double isPoissonInInjectionVeto(const Particle*)
    {
      StoreObjPtr<TRGSummary> trg;
      if (!trg)
        return Const::doubleNaN;
      return trg->isPoissonInInjectionVeto();
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

    bool passesAnyHighLevelTrigger(const Particle* p)
    {
      // for HLT, a c_accept is a pass and all other cases are fail
      // see mdst/dataobjects/include/SoftwareTriggerResult.h
      std::vector<std::string> hardcodedname
        = { "software_trigger_cut&filter&total_result" };
      double swtcr = std::get<double>(softwareTriggerResult(hardcodedname)(p));
      if (swtcr > 0.5) return true; // 1
      else             return false; // 0 or -1
    }

    Manager::FunctionPtr softwareTriggerPrescaling(const std::vector<std::string>& args)
    {
      /* The analyst has to know the name of the trigger she wants
       * (after having looked this up from the trigger db payload)
       *
       * This workflow will probably improve later: but for now parse the args
       * to check we have one name, then check the database object is valid.
       * If not return NAN.
       */
      if (args.size() != 1)
        B2FATAL("Wrong number of arguments for the function softwareTriggerPrescaling");
      std::string triggerIdentifier = args[0];

      auto outputFunction = [triggerIdentifier](const Particle*) -> double {

        DBObjPtr<DBRepresentationOfSoftwareTriggerCut> downloadedCut(fullFormatIdentifier(triggerIdentifier));
        if (not downloadedCut)
          return Const::doubleNaN;
        return double(downloadedCut->getPreScaleFactor());
      };

      return outputFunction;
    }

    //-------------------------------------------------------------------------
    VARIABLE_GROUP("L1 Trigger");
    REGISTER_VARIABLE("L1Trigger", L1Trigger,
                      "[Eventbased] Returns 1 if at least one PSNM L1 trigger bit is true.");
    REGISTER_METAVARIABLE("L1PSNM(name)", L1PSNM,
                          R"DOC(
[Eventbased] Returns the PSNM (Prescale And Mask, after prescale) status (1 or 0) of the output trigger bit with the given name. 
For some output trigger bits, we assign a prescale factor to reduce the number of triggered events. 
For example, we want to keep only 1% of Bhabha events. A prescale factor of 100 is then assigned to ``bha_3D`` (Bhabha selected in 3D criteria). 
Prescale factor of a given output trigger bit could be different in different datasets. 
It is recommended to use prescaled trigger bits (L1PSNM) or un-prescaled trigger bits (L1FTDL) for your analysis. 
In run-independent MC, configuration of the prescales in TSIM (trigger simulation) can be different from data, so L1 FTDL is recommended. 
In run-dependent MC, configuration of the prescales in TSIM is consistent with data, so L1PSNM is recommended.
Please check on `the dedicated Confluence page <https://confluence.desy.de/display/BI/TriggerBitTable>`__ or or `the dedicated Belle II notes <https://docs.belle2.org/search?ln=en&p=%22Trigger+Summary%22&f=&action_search=Search&c=Belle+II+Notes>`__ to find out the definition of trigger bits.
)DOC",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("L1FTDL(name)", L1FTDL,
                          R"DOC(
[Eventbased] Returns the FTDL (Final Trigger Decision Logic, before prescale) status (1 or 0) of the output trigger bit with the given name. Output bits are the outputs of GDL, combining different input trigger bits for final decision. For example, ``ty_0/1/2/3`` is one of the input trigger bits meaning the number of neuro 3D tracks is one/two/three/more than three. While ``yyy`` is one of the output trigger bits meaning ``(ty_2 or ty_3) and !veto``. Please check on `the dedicated Confluence page <https://confluence.desy.de/display/BI/TriggerBitTable>`__ or or `the dedicated Belle II notes <https://docs.belle2.org/search?ln=en&p=%22Trigger+Summary%22&f=&action_search=Search&c=Belle+II+Notes>`__ to find out the definition of trigger bits.
)DOC",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("L1Input(name)", L1Input,
                          R"DOC(
[Eventbased] Returns the input bit status (1 or 0) of the trigger bit with the given name. Input trigger bits are predefined selections from each sub-detector, with adjustment of the delay and width, in order to fix latency on GDL. For example, ``ty_0/1/2/3`` is one of the input trigger bits meaning the number of neuro 3D tracks is one/two/three/more than three. Please check on `the dedicated Confluence page <https://confluence.desy.de/display/BI/TriggerBitTable>`__ or or `the dedicated Belle II notes <https://docs.belle2.org/search?ln=en&p=%22Trigger+Summary%22&f=&action_search=Search&c=Belle+II+Notes>`__ to find out the definition of trigger bits.
)DOC", 
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("L1Prescale(name)", L1PSNMPrescale,
                          R"DOC(
[Eventbased] Returns the PSNM (prescale and mask) prescale factor of the trigger bit with the given name. Definition of prescale factor is shown in a few lines before in `L1PSNM`. Prescale factors are usually dependent on different datasets.
)DOC",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("L1PSNMBit(i)", L1PSNMBit,
                      R"DOC(
[Eventbased] Returns the PSNM (Prescale And Mask, after prescale) status (1 or 0) of i-th trigger bit.

.. warning::
  It is recommended to use this variable only for debugging and to use :b2:var:`L1PSNM`
  with the explicit trigger bit name for physics analyses or performance studies.
)DOC");
    REGISTER_VARIABLE("L1FTDLBit(i)", L1FTDLBit,
                      R"DOC(
[Eventbased] Returns the FTDL (Final Trigger Decision Logic, before prescale) status (1 or 0) of i-th trigger bit.

.. warning::
  It is recommended to use this variable only for debugging and to use :b2:var:`L1FTDL`
  with the explicit trigger bit name for physics analyses or performance studies.
)DOC");
    REGISTER_VARIABLE("L1InputBit(i)", L1InputBit,
                      R"DOC(
[Eventbased] Returns the input bit status (1 or 0) of the i-th input trigger bit.

.. warning::
  It is recommended to use this variable only for debugging and to use :b2:var:`L1Input`
  with the explicit trigger bit name for physics analyses or performance studies.
)DOC");
    REGISTER_VARIABLE("L1PSNMBitPrescale(i)", L1PSNMBitPrescale,
                      R"DOC(
[Eventbased] Returns the PSNM (prescale and mask) prescale of i-th trigger bit.

.. warning::
  It is recommended to use this variable only for debugging and to use :b2:var:`L1Prescale`
  with the explicit trigger bit name for physics analyses or performance studies.
)DOC");
    REGISTER_VARIABLE("L1TimeType", L1TimeType,
                      "[Eventbased] Returns kind of detector which determines the Level1 trigger timing. 0:ECL, 1:TOP, 2:SELF(timing of PSNM bit), 3:CDC, 5:delayed bhabha, 7: random, 13:poisson.");
    REGISTER_VARIABLE("L1TimeQuality", L1TimeQuality,
                      "[Eventbased] Returns expected Level1 timing resolution. This flag will be used for SVD 3-point sampling in future. 0:None; 1:Coarse (sigma > x ns); 2:FINE (sigma < x ns); x has been set to about 5ns before LS1 but can be changed in future");
    REGISTER_VARIABLE("isPoissonTriggerInInjectionVeto", isPoissonInInjectionVeto,
                      "[Eventbased] Returns 1 if the poisson random trigger is within the injection veto window.");
    //-------------------------------------------------------------------------
    VARIABLE_GROUP("Software Trigger");
    REGISTER_METAVARIABLE("SoftwareTriggerResult(triggerIdentifier)", softwareTriggerResult, R"DOC(
[Eventbased] [Expert] returns the SoftwareTriggerCutResult, defined as reject (-1), accept (1), or noResult (0). 
If the trigger identifier is not found, returns NaN.

For example:

.. code-block:: 

    SoftwareTriggerResult(filter 1_Estargt1_GeV_cluster_no_other_cluster_Estargt0.3_GeV)

which is equivalent to

.. code-block::

    SoftwareTriggerResult(software_trigger_cut&filter&1_Estargt1_GeV_cluster_no_other_cluster_Estargt0.3_GeV)


.. warning:: the meanings of these change depending if using trigger or the skim stage, hence expert.

.. seealso:: ``b2hlt_triggers`` for possible triggerIdentifiers.

        )DOC", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("SoftwareTriggerResultNonPrescaled(triggerIdentifier)", softwareTriggerResultNonPrescaled,
                          "[Eventbased] [Expert] returns the SoftwareTriggerCutResult, "
                          "if this trigger would not be prescaled."
                          "Please note, this is not the final HLT decision! "
                          "It is defined as reject (-1), accept (1), or noResult (0). Note "
                          "that the meanings of these change depending if using trigger "
                          "or the skim stage, hence expert."
                          "If the trigger identifier is not found, returns NaN.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("HighLevelTrigger", passesAnyHighLevelTrigger,
                      "[Eventbased] True if event passes the HLT trigger, false if not");
    REGISTER_METAVARIABLE("SoftwareTriggerPrescaling(triggerIdentifier)", softwareTriggerPrescaling,
                          "[Eventbased] return the prescaling for the specific software trigger identifier. "
                          "Please note, this prescaling is taken from the currently setup database. It only corresponds "
                          "to the correct HLT prescale if you are using the online database!"
                          "If the trigger identifier is not found, returns NaN.", Manager::VariableDataType::c_double);
    //-------------------------------------------------------------------------
  }
}
