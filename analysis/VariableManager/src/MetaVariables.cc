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
#include <framework/utilities/MakeROOTCompatible.h>

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
#include <memory>
#include <string>

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

    Manager::FunctionPtr nCleanedTracks(const std::vector<std::string>& arguments)
    {
      if (arguments.size() <= 1) {

        std::string cutString;
        if (arguments.size() == 1)
          cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
        auto func = [cut](const Particle*) -> double {

          unsigned int number_of_tracks = 0;
          StoreArray<Track> tracks;
          for (const auto& track : tracks)
          {
            const TrackFitResult* trackFit = track.getTrackFitResult(Const::pion);
            if (trackFit->getChargeSign() == 0) {
              // Ignore track
            } else {
              Particle particle(&track, Const::pion);
              if (cut->check(&particle))
                number_of_tracks++;
            }
          }

          return static_cast<double>(number_of_tracks);

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function nCleanedTracks");
      }
    }

    Manager::FunctionPtr nCleanedECLClusters(const std::vector<std::string>& arguments)
    {
      if (arguments.size() <= 1) {

        std::string cutString;
        if (arguments.size() == 1)
          cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
        auto func = [cut](const Particle*) -> double {

          unsigned int number_of_clusters = 0;
          StoreArray<ECLCluster> clusters;
          for (const auto& cluster : clusters)
          {
            Particle particle(&cluster);
            if (cut->check(&particle))
              number_of_clusters++;
          }

          return static_cast<double>(number_of_clusters);

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function nCleanedECLClusters");
      }
    }

    Manager::FunctionPtr passesCut(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
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

    Manager::FunctionPtr varFor(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int pdgCode = 0;
        try {
          pdgCode = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The first argument of varFor meta function must be a positive integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [pdgCode, var](const Particle * particle) -> double {

          if (std::abs(particle -> getPDGCode()) == std::abs(pdgCode))
            return var -> function(particle);
          else return -999;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function varFor");
      }
    }

    Manager::FunctionPtr isDaughterOfList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {
        auto listNames = arguments;
        auto func = [listNames](const Particle * particle) -> double {
          double output = 0;

          for (auto& iListName : listNames)
          {

            StoreObjPtr<ParticleList> listOfParticles(iListName);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << iListName << " given to isDaughterOfList");

            for (unsigned i = 0; i < listOfParticles->getListSize(); ++i) {
              Particle* iParticle = listOfParticles->getParticle(i);
              for (unsigned j = 0; j < iParticle->getNDaughters(); ++j) {
                if (particle == iParticle->getDaughter(j)) {
                  output = 1; goto endloop;
                }
              }
            }
          }
endloop:
          return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isDaughterOfList");
      }
    }

    Manager::FunctionPtr isGrandDaughterOfList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {
        auto listNames = arguments;
        auto func = [listNames](const Particle * particle) -> double {
          double output = 0;

          for (auto& iListName : listNames)
          {

            StoreObjPtr<ParticleList> listOfParticles(iListName);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << iListName << " given to isGrandDaughterOfList");

            for (unsigned i = 0; i < listOfParticles->getListSize(); ++i) {
              Particle* iParticle = listOfParticles->getParticle(i);
              for (unsigned j = 0; j < iParticle->getNDaughters(); ++j) {
                const Particle* jDaughter = iParticle->getDaughter(j);
                for (unsigned k = 0; k < jDaughter->getNDaughters(); ++k) {

                  if (particle == jDaughter->getDaughter(k)) {
                    output = 1; goto endloop;
                  }
                }
              }
            }

          }
endloop:
          return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isGrandDaughterOfList");
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

    Manager::FunctionPtr daughterLowest(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double min = -999;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            double iValue = var->function(particle->getDaughter(j));
            if (iValue == -999) continue;
            if (min == -999) min = iValue;
            if (iValue < min) min = iValue;
          }
          return min;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterLowest");
      }
    }

    Manager::FunctionPtr daughterHighest(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double max = -999;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            double iValue = var->function(particle->getDaughter(j));
            if (iValue == -999) continue;
            if (iValue > max) max = iValue;
          }
          return max;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterHighest");
      }
    }

    Manager::FunctionPtr daughterDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First two arguments of daughterDiffOf meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[2]);
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else {
            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            return diff;}
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOf");
      }
    }

    Manager::FunctionPtr daughterNormDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First two arguments of daughterDiffOf meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[2]);
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else {
            double iValue = var->function(particle->getDaughter(iDaughterNumber));
            double jValue = var->function(particle->getDaughter(jDaughterNumber));
            return (jValue - iValue) / (jValue + iValue);}
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterNormDiffOf");
      }
    }

    Manager::FunctionPtr daughterAngleInBetween(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 || arguments.size() == 3) {
        std::vector<int> daughterIndices;
        try {
          for (auto& argument : arguments) daughterIndices.push_back(Belle2::convertString<int>(argument));
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The arguments of daughterAngleInBetween meta function must be integers!");
          return nullptr;
        }
        auto func = [daughterIndices](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (daughterIndices.size() == 2)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters()))
              return -999;
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              TVector3 pi = frame.getMomentum(particle->getDaughter(daughterIndices[0])).Vect();
              TVector3 pj = frame.getMomentum(particle->getDaughter(daughterIndices[1])).Vect();
              return pi.Angle(pj);
            }
          } else if (daughterIndices.size() == 3)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters())
                || daughterIndices[2] >= int(particle->getNDaughters())) return -999;
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              TVector3 pi = frame.getMomentum(particle->getDaughter(daughterIndices[0])).Vect();
              TVector3 pj = frame.getMomentum(particle->getDaughter(daughterIndices[1])).Vect();
              TVector3 pk = frame.getMomentum(particle->getDaughter(daughterIndices[2])).Vect();
              return pk.Angle(pi + pj);
            }
          } else return -999;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterAngleInBetween");
      }
    }

    Manager::FunctionPtr daughterInvM(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 1) {
        std::vector<int> daughterIndices;
        try {
          for (auto& argument : arguments) daughterIndices.push_back(Belle2::convertString<int>(argument));
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The arguments of daughterInvM meta function must be integers!");
          return nullptr;
        }
        auto func = [daughterIndices](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          else {
            const auto& frame = ReferenceFrame::GetCurrent();
            TLorentzVector pSum;

            for (auto& index : daughterIndices)
            {
              if (index >= int(particle->getNDaughters())) {
                return -999; break;
              } else pSum += frame.getMomentum(particle->getDaughter(index));
            }

            return pSum.M();}
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterInvM. At least two integers are needed.");
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

        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));

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
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));

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
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
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

    Manager::FunctionPtr numberOfNonOverlappingParticles(const std::vector<std::string>& arguments)
    {

      auto func = [arguments](const Particle * particle) -> double {

        unsigned _numberOfNonOverlappingParticles = 0;
        for (const auto& listName : arguments)
        {
          StoreObjPtr<ParticleList> list(listName);
          if (not list.isValid()) {
            B2FATAL("Invalid list named " << listName << " encountered in numberOfNonOverlappingParticles.");
          }
          for (unsigned int i = 0; i < list->getListSize(); i++) {
            const Particle* p = list->getParticle(i);
            if (not particle->overlapsWith(p)) {
              _numberOfNonOverlappingParticles++;
            }
          }
        }
        return _numberOfNonOverlappingParticles;
      };

      return func;

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
    REGISTER_VARIABLE("nCleanedECLClusters(cut)", nCleanedECLClusters,
                      "[Eventbased] ]Returns the number of clean Clusters in the event\n"
                      "Clean clusters are defined by the clusters which pass the given cut assuming a photon hypothesis.");
    REGISTER_VARIABLE("nCleanedTracks(cut)", nCleanedTracks,
                      "[Eventbased] ]Returns the number of clean Tracks in the event\n"
                      "Clean tracks are defined by the tracks which pass the given cut assuming a pion hypothesis.");
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
    REGISTER_VARIABLE("varFor(pdgCode, variable)", varFor,
                      "Returns the value of the variable for the given particle if its abs(pdgCode) agrees with the given one.\n"
                      "E.g. varFor(11, p) returns the momentum if the particle is an electron or a positron.");
    REGISTER_VARIABLE("isDaughterOfList(particleListNames)", isDaughterOfList,
                      "Returns 1 if the given particle is a daughter of at least one of the particles of the given particle Lists.");
    REGISTER_VARIABLE("isGrandDaughterOfList(particleListNames)", isGrandDaughterOfList,
                      "Returns 1 if the given particle is a grand daughter of at least one of the particles of the given particle Lists.");
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
    REGISTER_VARIABLE("daughterLowest(variable)", daughterLowest,
                      "Returns the lowest value of the given variable among all daughters.\n"
                      "E.g. useCMSFrame(daughterLowest(p)) returns the lowest momentum in CMS frame.");
    REGISTER_VARIABLE("daughterHighest(variable)", daughterHighest,
                      "Returns the highest value of the given variable among all daughters.\n"
                      "E.g. useCMSFrame(daughterHighest(p)) returns the highest momentum in CMS frame.");
    REGISTER_VARIABLE("daughterDiffOf(i, j, variable)", daughterDiffOf,
                      "Returns the difference of a variable between the two given daughters.\n"
                      "E.g. useRestFrame(daughterDiffOf(0, 1, p)) returns the momentum difference between first and second daughter in the rest frame of the given particle.");
    REGISTER_VARIABLE("daughterNormDiffOf(i, j, variable)", daughterNormDiffOf,
                      "Returns the normalized difference of a variable between the two given daughters.\n"
                      "E.g. daughterNormDiffOf(0, 1, p) returns the normalized momentum difference between first and second daughter in the lab frame.");
    REGISTER_VARIABLE("daughterAngleInBetween(i, j)", daughterAngleInBetween,
                      "If two indices given: Variable returns the angle between the momenta of the two given daughters.\n"
                      "If three indices given: Variable returns the angle between the momentum of the third particle and a vector "
                      "which is the sum of the first two daughter momenta.\n"
                      "E.g. useLabFrame(daughterAngleInBetween(0, 1)) returns the angle between first and second daughter in the Lab frame.");
    REGISTER_VARIABLE("daughterInvM(i, j)", daughterInvM,
                      "Returns the invariant Mass adding the Lorentz vectors of the given daughters.\n"
                      "E.g. daughterInvM(0, 1, 2) returns the invariant Mass m = sqrt((p0 + p1 + p2)^2) of first, second and third daughter.");
    REGISTER_VARIABLE("extraInfo(name)", extraInfo,
                      "Returns extra info stored under the given name.\n"
                      "The extraInfo has to be set first by a module like MVAExpert. If nothing is set under this name, -999 is returned.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the MVAExpert.");
    REGISTER_VARIABLE("eventExtraInfo(name)", eventExtraInfo,
                      "[eventbased] Returns extra info stored under the given name in the event extra info.\n"
                      "The extraInfo has to be set first by another module like MVAExpert in event mode.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the MVAExpert for an event.");
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

    REGISTER_VARIABLE("numberOfNonOverlappingParticles(pList1, pList2, ...)", numberOfNonOverlappingParticles,
                      "Returns the number of non-overlapping particles in the given particle lists"
                      "Useful to check if there is additional physics going on in the detector if one reconstructed the Y4S");

  }
}
