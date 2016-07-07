/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VariableManager/MetaVariables.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/utility/EvtPDLUtil.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Conversion.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <boost/lexical_cast.hpp>
#include <limits>

#include <cmath>
#include <stdexcept>

namespace Belle2 {
  namespace Variable {

    Manager::FunctionPtr useRestFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          UseReferenceFrame<RestFrame> frame(particle);
          double result = var->function(particle);
          return result;
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function useRestFrame");
        return nullptr;
      }
    }

    Manager::FunctionPtr useCMSFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          UseReferenceFrame<CMSFrame> frame;
          double result = var->function(particle);
          return result;
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function useCMSFrame");
        return nullptr;
      }
    }

    Manager::FunctionPtr useLabFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          UseReferenceFrame<LabFrame> frame;
          double result = var->function(particle);
          return result;
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function useLabFrame");
        return nullptr;
      }
    }

    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            StoreObjPtr<EventExtraInfo> eventExtraInfo;
            return eventExtraInfo->getExtraInfo(extraInfoName);
          }
          try {
            return particle->getExtraInfo(extraInfoName);
          } catch (const std::runtime_error& error)
          {
            return -999.;
          }
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function extraInfo");
        return nullptr;
      }
    }

    Manager::FunctionPtr eventExtraInfo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName](const Particle*) -> double {
          StoreObjPtr<EventExtraInfo> eventExtraInfo;
          return eventExtraInfo->getExtraInfo(extraInfoName);
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function extraInfo");
        return nullptr;
      }
    }

    Manager::FunctionPtr eventCached(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        std::string key = std::string("__") + makeROOTCompatible(var->name);
        auto func = [var, key](const Particle*) -> double {

          StoreObjPtr<EventExtraInfo> eventExtraInfo;
          if (not eventExtraInfo.isValid())
            eventExtraInfo.create();
          if (eventExtraInfo->hasExtraInfo(key))
          {
            return eventExtraInfo->getExtraInfo(key);
          } else {
            double value = var->function(nullptr);
            eventExtraInfo->addExtraInfo(key, value);
            return value;
          }
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function eventCached");
        return nullptr;
      }
    }

    Manager::FunctionPtr particleCached(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        std::string key = std::string("__") + makeROOTCompatible(var->name);
        auto func = [var, key](const Particle * particle) -> double {

          if (particle->hasExtraInfo(key))
          {
            return particle->getExtraInfo(key);
          } else {
            double value = var->function(particle);
            // Remove constness from Particle pointer.
            // The extra-info is used as a cache in our case,
            // indicated by the double-underscore in front of the key.
            // One could implement the cache as a separate property of the particle object
            // and mark it as mutable, however, this would only lead to code duplication
            // and an increased size of the particle object.
            // Thus, we decided to use the extra-info field and cast away the const in this case.
            const_cast<Particle*>(particle)->addExtraInfo(key, value);
            return value;
          }
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function particleCached");
        return nullptr;
      }
    }

    Manager::FunctionPtr formula(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        char operation = ' ';
        int pos = 0;

        if (arguments[0].find('+') != std::string::npos) {
          pos = arguments[0].find('+');
          operation = '+';
        } else if (arguments[0].find('-') != std::string::npos) {
          pos = arguments[0].find('-');
          operation = '-';
        } else if (arguments[0].find('*') != std::string::npos) {
          pos = arguments[0].find('*');
          operation = '*';
        } else if (arguments[0].find('/') != std::string::npos) {
          pos = arguments[0].find('/');
          operation = '/';
        } else if (arguments[0].find('^') != std::string::npos) {
          pos = arguments[0].find('^');
          operation = '^';
        }

        if (operation != ' ') {
          std::string lhs = std::string("formula(") + arguments[0].substr(0, pos) + std::string(")");
          std::string rhs = std::string("formula(") + arguments[0].substr(pos + 1) + std::string(")");

          const Variable::Manager::Var* var = Manager::Instance().getVariable(lhs);
          const Variable::Manager::Var* var2 = Manager::Instance().getVariable(rhs);

          auto func = [var, operation, var2](const Particle * particle) -> double {
            switch (operation)
            {
              case '+': return var->function(particle) + var2->function(particle);
              case '-': return var->function(particle) - var2->function(particle);
              case '*': return var->function(particle) * var2->function(particle);
              case '/': return var->function(particle) / var2->function(particle);
              case '^': return std::pow(var->function(particle), var2->function(particle));
            }
            return 0;
          };
          return func;
        } else {
          const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
          return var->function;
        }
      } else {
        B2FATAL("Wrong number of arguments for meta function formula");
      }
    }

    Manager::FunctionPtr passesCut(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(cutString));
        auto func = [cut](const Particle * particle) -> double {

          if (particle == nullptr)
            return -999;
          if (cut->check(particle))
            return 1;
          else
            return 0;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function passesCut");
      }
    }

    Manager::FunctionPtr daughterProductOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double product = 1.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            product *= var->function(particle->getDaughter(j));
          }
          return product;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterProductOf");
      }
    }

    Manager::FunctionPtr daughterSumOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double sum = 0.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            sum += var->function(particle->getDaughter(j));
          }
          return sum;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterSumOf");
      }
    }

    Manager::FunctionPtr modulo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        int divideBy = 1;
        try {
          divideBy = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("Second argument of modulo  meta function must be integer!");
          return nullptr;
        }
        auto func = [var, divideBy](const Particle * particle) -> double { return int(var->function(particle)) % divideBy; };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function modulo");
      }
    }

    Manager::FunctionPtr isNAN(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        auto func = [var](const Particle * particle) -> double { return std::isnan(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isNAN");
      }
    }

    Manager::FunctionPtr isInfinity(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        auto func = [var](const Particle * particle) -> double { return std::isinf(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isInfinity");
      }
    }

    Manager::FunctionPtr abs(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::abs(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function abs");
      }
    }

    Manager::FunctionPtr sin(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::sin(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function sin");
      }
    }

    Manager::FunctionPtr cos(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::cos(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function sin");
      }
    }

    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of daughter meta function must be integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (daughterNumber >= int(particle->getNDaughters()))
            return -999;
          else
            return var->function(particle->getDaughter(daughterNumber));
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughter");
      }
    }

    Manager::FunctionPtr getVariableByRank(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 4) {
        std::string listName = arguments[0];
        std::string rankedVariableName = arguments[1];
        std::string returnVariableName = arguments[2];
        std::string extraInfoName = rankedVariableName + "_rank";
        int rank = 1;
        try {
          rank = Belle2::convertString<int>(arguments[3]);
        } catch (boost::bad_lexical_cast&)  {
          B2ERROR("3rd argument of getVariableByRank meta function (Rank) must be integer!");
          return nullptr;
        }

        const Variable::Manager::Var* var = Manager::Instance().getVariable(returnVariableName);
        auto func = [var, rank, extraInfoName, listName](const Particle*)-> double {
          StoreObjPtr<ParticleList> list(listName);

          const unsigned int numParticles = list->getListSize();
          for (unsigned int i = 0; i < numParticles; i++)
          {
            const Particle* p = list->getParticle(i);
            if (p->getExtraInfo(extraInfoName) == rank)
              return var->function(p);
          }
          // return 0;
          return std::numeric_limits<double>::signaling_NaN();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function getVariableByRank");
      }
    }

    Manager::FunctionPtr countInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1 or arguments.size() == 2) {

        std::string listName = arguments[0];
        std::string cutString = "";

        if (arguments.size() == 2) {
          cutString = arguments[1];
        }

        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(cutString));

        auto func = [listName, cut](const Particle*) -> double {

          StoreObjPtr<ParticleList> list(listName);
          double sum = 0;
          for (unsigned int i = 0; i < list->getListSize(); i++)
          {
            const Particle* particle = list->getParticle(i);
            if (cut->check(particle)) {
              sum++;
            }
          }
          return sum;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function countInList");
      }
    }

    Manager::FunctionPtr veto(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 or arguments.size() == 3) {

        std::string roeListName = arguments[0];
        std::string cutString = arguments[1];
        int pdgCode = 11;
        if (arguments.size() == 2) {
          B2INFO("Use pdgCode 11 as default in meta variable veto, other arguments: " << roeListName << ", " << cutString);
        } else {
          try {
            pdgCode = Belle2::convertString<int>(arguments[2]);;
          } catch (boost::bad_lexical_cast&) {
            B2WARNING("Third argument of veto meta function must be integer!");
            return nullptr;
          }
        }

        auto flavourType = (Belle2::EvtPDLUtil::hasAntiParticle(pdgCode)) ? Particle::c_Flavored : Particle::c_Unflavored;
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(cutString));

        auto func = [roeListName, cut, pdgCode, flavourType](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          StoreObjPtr<ParticleList> roeList(roeListName);
          TLorentzVector vec = particle->get4Vector();
          for (unsigned int i = 0; i < roeList->getListSize(); i++)
          {
            const Particle* roeParticle = roeList->getParticle(i);
            if (not particle->overlapsWith(roeParticle)) {
              TLorentzVector tempCombination = roeParticle->get4Vector() + vec;
              std::vector<int> indices = { particle->getArrayIndex(), roeParticle->getArrayIndex() };
              Particle tempParticle = Particle(tempCombination, pdgCode, flavourType, indices, particle->getArrayPointer());
              if (cut->check(&tempParticle)) {
                return 1;
              }
            }
          }
          return 0;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function veto");
      }
    }

    Manager::FunctionPtr countDaughters(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(cutString));
        auto func = [cut](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          unsigned int n = 0;
          for (auto& daughter : particle->getDaughters())
          {
            if (cut->check(daughter))
              ++n;
          }
          return n;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function countDaughters");
      }
    }


    Manager::FunctionPtr NBDeltaIfMissing(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        Const::PIDDetectorSet set = Const::TOP;
        if (arguments[0] == "TOP") {
          set = Const::TOP;
        } else if (arguments[0] == "ARICH") {
          set = Const::ARICH;
        } else {
          B2FATAL("Encountered unsupported sub-detector type " + arguments[0] + " in NBDeltaIfMissing only TOP and ARICH are supported."
                  "Note that other sub-detectors providing PID information like ECL, dEdx do not have missing values!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, set](const Particle * particle) -> double {
          const PIDLikelihood* pid = particle->getPIDLikelihood();
          if (!pid)
            return -999;
          if (not pid->isAvailable(set))
            return -999;
          return var->function(particle);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function NBDeltaIfMissing");
      }
    }


    Manager::FunctionPtr matchedMC(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          const MCParticle* mcp = particle->getRelated<MCParticle>();
          if (!mcp)
            return -999;

          Particle tmpPart(mcp);
          return var->function(&tmpPart);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function matchedMC");
      }
    }

    Manager::FunctionPtr matchedMCHasPDG(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        int inputPDG = 0 ;
        try {
          inputPDG = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2ERROR("Argument must be an integer value.");
        }

        auto func = [inputPDG](const Particle * particle) -> double{
          const MCParticle* mcp = particle->getRelated<MCParticle>();
          if (!mcp)
            return 0.5;

          if (std::abs(mcp->getPDG()) == inputPDG)
          {
            return 1;
          } else
            return 0;

        };
        return func;
      } else
        B2FATAL("Wrong number of arguments for meta function matchedMC");
    }


    VARIABLE_GROUP("MetaFunctions");
    REGISTER_VARIABLE("formula(v1 + v2 * v3 - v4 / v5^v6)", formula,
                      "Returns the result of the given formula, where v1-v6 are variables.\n"
                      "Useful Calculate formula, no parenthesis allowed yet.");
    REGISTER_VARIABLE("useRestFrame(variable)", useRestFrame,
                      "Returns the value of the variable using the rest frame of the given particle as current reference frame.\n"
                      "E.g. useRestFrame(daughter(0, p)) returns the total momentum of the first daughter in its mother's rest-frame");
    REGISTER_VARIABLE("useCMSFrame(variable)", useCMSFrame,
                      "Returns the value of the variable using the CMS frame as current reference frame.\n"
                      "E.g. useCMSFrame(E) returns the energy of a particle in the CMS frame.");
    REGISTER_VARIABLE("useLabFrame(variable)", useLabFrame,
                      "Returns the value of the variable using the lab frame as current reference frame.\n"
                      "The lab frame is the default reference frame, usually you don't need to use this meta-variable.\n"
                      "E.g. useLabFrame(E) returns the energy of a particle in the Lab frame, same as just E.\n"
                      "     useRestFrame(daughter(0, formula(E - useLabFrame(E)))) only corner-cases like this need to use this variable.");
    REGISTER_VARIABLE("passesCut(cut)", passesCut,
                      "Returns 1 if particle passes the cut otherwise 0.\n"
                      "Useful if you want to write out if a particle would have passed a cut or not.\n"
                      "Returns -999 if particle is a nullptr.");
    REGISTER_VARIABLE("countDaughters(cut)", countDaughters,
                      "Returns number of direct daughters which satisfy the cut.\n"
                      "Used by the skimming package (for what exactly?)\n"
                      "Returns -999 if particle is a nullptr.");
    REGISTER_VARIABLE("daughter(i, variable)", daughter,
                      "Returns value of variable for the i-th daughter."
                      "E.g. daughter(0, p) returns the total momentum of the first daughter.\n"
                      "     daughter(0, daughter(1, p) returns the total momentum of the second daughter of the first daughter.\n"
                      "Returns -999 if particle is nullptr or if the given daughter-index is out of bound (>= amount of daughters).");
    REGISTER_VARIABLE("daughterProductOf(variable)", daughterProductOf,
                      "Returns product of a variable over all daughters.\n"
                      "E.g. daughterProductOf(extraInfo(SignalProbability)) returns the product of the SignalProbabilitys of all daughters.");
    REGISTER_VARIABLE("daughterSumOf(variable)", daughterSumOf,
                      "Returns sum of a variable over all daughters.\n"
                      "E.g. daughterSumOf(nDaughters) returns the number of grand-daughters.");
    REGISTER_VARIABLE("extraInfo(name)", extraInfo,
                      "Returns extra info stored under the given name.\n"
                      "The extraInfo has to be set first by a module like TMVAExpert. If nothing is set under this name, -999 is returned.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the TMVAExpert.");
    REGISTER_VARIABLE("eventExtraInfo(name)", eventExtraInfo,
                      "[eventbased] Returns extra info stored under the given name in the event extra info.\n"
                      "The extraInfo has to be set first by another module like TMVAExpert in event mode.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the TMVAExpert for an event.");
    REGISTER_VARIABLE("eventCached(variable)", eventCached,
                      "[eventbased] Returns value of event-based variable and caches this value in the EventExtraInfo.\n"
                      "The result of second call to this variable in the same event will be provided from the cache.");
    REGISTER_VARIABLE("particleCached(variable)", particleCached,
                      "Returns value of given variable and caches this value in the ParticleExtraInfo of the provided particle.\n"
                      "The result of second call to this variable on the same particle will be provided from the cache.");
    REGISTER_VARIABLE("modulo(variable, n)", modulo,
                      "Returns rest of division of variable by n.");
    REGISTER_VARIABLE("abs(variable)", abs,
                      "Returns absolute value of the given variable.\n"
                      "E.g. abs(mcPDG) returns the absolute value of the mcPDG, which is often useful for cuts.");
    REGISTER_VARIABLE("sin(variable)", sin,
                      "Returns sin value of the given variable.\n"
                      "E.g. sin(?) returns the sine of the value of the variable.");
    REGISTER_VARIABLE("cos(variable)", cos,
                      "Returns cos value of the given variable.\n"
                      "E.g. sin(?) returns the cosine of the value of the variable.");
    REGISTER_VARIABLE("isNAN(variable)", isNAN,
                      "Returns true if variable value evaluates to nan (determined via std::isnan(double)).\n"
                      "Useful for debugging.");
    REGISTER_VARIABLE("isInfinity(variable)", isInfinity,
                      "Returns true if variable value evaluates to infinity (determined via std::isinf(double)).\n"
                      "Useful for debugging.");
    REGISTER_VARIABLE("NBDeltaIfMissing(detector, variable)", NBDeltaIfMissing,
                      "Returns value of variable if pid information of detector is available otherwise -999 (delta function of NeuroBayes).\n"
                      "Possible values for detector are TOP and ARICH.\n"
                      "Useful as a feature variable for the NeuroBayes classifier, which tags missing values with -999.\n"
                      "E.g. NBDeltaIfMissing(TOP, eid_TOP) returns electron id of TOP if this information is available, otherwise -999.\n"
                      "     NBDeltaIfMissing(ARICH, Kid_ARICH) returns kaon id of ARICH if this information is available, otherwise -999.");
    REGISTER_VARIABLE("veto(particleList, cut, pdgCode = 11)", veto,
                      "Combines current particle with particles from the given particle list and returns 1 if the combination passes the provided cut. \n"
                      "For instance one can apply this function on a signal Photon and provide a list of all photons in the rest of event and a cut \n"
                      "around the neutral Pion mass (e.g. 0.130 < M < 0.140). \n"
                      "If a combination of the signal Photon with a ROE photon fits this criteria, hence looks like a neutral pion, the veto-Metavariable will return 1");
    REGISTER_VARIABLE("matchedMC(variable)", matchedMC,
                      "Returns variable output for the matched MCParticle by constructing a temporary Particle from it.\n"
                      "This may not work too well if your variable requires accessing daughters of the particle.\n"
                      "E.g. matchedMC(p) returns the total momentum of the related MCParticle.\n"
                      "Returns -999 if no matched MCParticle exists.");
    REGISTER_VARIABLE("countInList(particleList, cut='')", countInList,
                      "Returns number of particle which pass given in cut in the specified particle list.\n"
                      "Useful for creating statistics about the number of particles in a list.\n"
                      "E.g. countInList(e+, isSignal == 1) returns the number of correctly reconstructed electrons in the event.\n"
                      "The variable is event-based and does not need a valid particle pointer as input.");
    REGISTER_VARIABLE("getVariableByRank(particleList, rankedVariableName, variableName, rank)", getVariableByRank,
                      "Returns a specific variable according to its rank in a particle list.\n"
                      "The rank is determined via BestCandidateSelection. BestCandidateSelection has to be used before.");
    REGISTER_VARIABLE("matchedMCHasPDG(PDGCode)", matchedMCHasPDG,
                      "Returns if the absolute value of aPDGCode of a MCParticle related to a Particle matches a given PDGCode."
                      "Returns 0/0.5/1 if PDGCode does not match/is not available/ matches");

  }
}
