/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#include <analysis/variables/MetaVariables.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/variables/VariableFormulaConstructor.h>

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
#include <boost/algorithm/string.hpp>
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

    Manager::FunctionPtr useROERecoilFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
          if (!roe)
          {
            B2ERROR("Relation between particle and ROE doesn't exist!");
            return -999.;
          }
          PCmsLabTransform T;
          TLorentzVector pRecoil = T.getBeamParams().getHER() + T.getBeamParams().getLER() - roe->get4Vector();
          Particle tmp(pRecoil, 0);
          UseReferenceFrame<RestFrame> frame(&tmp);
          double result = var->function(particle);
          return result;
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function useROERecoilFrame");
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

    // Formula of other variables, going to require a space between all operators and operations.
    // Later can add some check for : (colon) trailing + or - to distinguish between particle lists
    // and operations, but for now cbf.
    Manager::FunctionPtr formula(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) B2FATAL("Wrong number of arguments for meta function formula");
      FormulaParser<VariableFormulaConstructor> parser;
      try {
        return parser.parse(arguments[0]);
      } catch (std::runtime_error& e) {
        B2FATAL(e.what());
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
            const TrackFitResult* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
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
            // look only at momentum of N1 (n photons) ECLClusters
            if (cluster.getHypothesisId() != ECLCluster::Hypothesis::c_nPhotons)
              continue;

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

    Manager::FunctionPtr passesEventCut(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
        auto func = [cut](const Particle*) -> double {
          if (cut->check(nullptr))
            return 1;
          else
            return 0;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function passesEventCut");
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

    Manager::FunctionPtr nParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle * particle) -> double {

          (void) particle;
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to nParticlesInList");

          return listOfParticles->getListSize();

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function nParticlesInList");
      }
    }

    Manager::FunctionPtr isInList(const std::vector<std::string>& arguments)
    {
      // unpack arguments, there should be only one: the name of the list we're checking
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments for isInList");
      }
      auto listName = arguments[0];

      auto func = [listName](const Particle * particle) -> double {

        // check the list exists
        StoreObjPtr<ParticleList> list(listName);
        if (!(list.isValid()))
        {
          B2FATAL("Invalid Listname " << listName << " given to isInList");
        }

        // is the particle in the list?
        bool isIn = list->contains(particle);
        return double(isIn);

      };
      return func;
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
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
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

    Manager::FunctionPtr daughterDiffOfPhi(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The two arguments of daughterDiffOfPhi meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable("phi");
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else
          {
            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            if (fabs(diff) > M_PI)
            {
              if (diff > M_PI) {
                diff = diff - 2 * M_PI;
              } else {
                diff = 2 * M_PI + diff;
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOfPhi");
      }
    }

    Manager::FunctionPtr daughterDiffOfClusterPhi(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The two arguments of daughterDiffOfClusterPhi meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable("clusterPhi");
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else
          {
            if (std::isnan(var->function(particle->getDaughter(iDaughterNumber))) or std::isnan(var->function(particle->getDaughter(jDaughterNumber))))
              return std::numeric_limits<float>::quiet_NaN();

            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            if (fabs(diff) > M_PI)
            {
              if (diff > M_PI) {
                diff = diff - 2 * M_PI;
              } else {
                diff = 2 * M_PI + diff;
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOfClusterPhi");
      }
    }


    Manager::FunctionPtr daughterDiffOfPhiCMS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The two arguments of daughterDiffOfPhi meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable("useCMSFrame(phi)");
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else
          {
            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            if (fabs(diff) > M_PI)
            {
              if (diff > M_PI) {
                diff = diff - 2 * M_PI;
              } else {
                diff = 2 * M_PI + diff;
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOfPhi");
      }
    }

    Manager::FunctionPtr daughterDiffOfClusterPhiCMS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The two arguments of daughterDiffOfClusterPhi meta function must be integers!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable("useCMSFrame(clusterPhi)");
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return -999;
          else
          {
            if (std::isnan(var->function(particle->getDaughter(iDaughterNumber))) or std::isnan(var->function(particle->getDaughter(jDaughterNumber))))
              return std::numeric_limits<float>::quiet_NaN();

            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            if (fabs(diff) > M_PI)
            {
              if (diff > M_PI) {
                diff = diff - 2 * M_PI;
              } else {
                diff = 2 * M_PI + diff;
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOfClusterPhi");
      }
    }

    Manager::FunctionPtr daughterNormDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        // have to tell cppcheck that these lines are fine, because it doesn't
        // support the lambda function syntax and throws a (wrong) variableScope

        // cppcheck-suppress variableScope
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          // cppcheck-suppress unreadVariable
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

    Manager::FunctionPtr daughterMotherDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of daughterMotherDiffOf meta function must be integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (daughterNumber >= int(particle->getNDaughters()))
            return -999;
          else {
            double diff = var->function(particle) - var->function(particle->getDaughter(daughterNumber));
            return diff;}
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterMotherDiffOf");
      }
    }

    Manager::FunctionPtr daughterMotherNormDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of daughterMotherDiffOf meta function must be integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (daughterNumber >= int(particle->getNDaughters()))
            return -999;
          else {
            double daughterValue = var->function(particle->getDaughter(daughterNumber));
            double motherValue = var->function(particle);
            return (motherValue - daughterValue) / (motherValue + daughterValue);}
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterMotherNormDiffOf");
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

    Manager::FunctionPtr daughterClusterAngleInBetween(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 || arguments.size() == 3) {
        std::vector<int> daughterIndices;
        try {
          for (auto& argument : arguments) daughterIndices.push_back(Belle2::convertString<int>(argument));
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The arguments of daughterClusterAngleInBetween meta function must be integers!");
          return nullptr;
        }
        auto func = [daughterIndices](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (daughterIndices.size() == 2)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters()))
              return -998;
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              const ECLCluster* clusteri = (particle->getDaughter(daughterIndices[0]))->getECLCluster();
              const ECLCluster* clusterj = (particle->getDaughter(daughterIndices[1]))->getECLCluster();
              if (clusteri and clusterj) {
                ClusterUtils clusutils;
                TVector3 pi = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusteri)).Vect();
                TVector3 pj = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterj)).Vect();
                return pi.Angle(pj);
              }
              return std::numeric_limits<float>::quiet_NaN();
            }
          } else if (daughterIndices.size() == 3)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters())
                || daughterIndices[2] >= int(particle->getNDaughters())) return -997;
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              const ECLCluster* clusteri = (particle->getDaughter(daughterIndices[0]))->getECLCluster();
              const ECLCluster* clusterj = (particle->getDaughter(daughterIndices[1]))->getECLCluster();
              const ECLCluster* clusterk = (particle->getDaughter(daughterIndices[2]))->getECLCluster();
              if (clusteri and clusterj and clusterk) {
                ClusterUtils clusutils;
                TVector3 pi = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusteri)).Vect();
                TVector3 pj = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterj)).Vect();
                TVector3 pk = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterk)).Vect();
                return pk.Angle(pi + pj);
              }
              return std::numeric_limits<float>::quiet_NaN();
            }
          } else return -996;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterClusterAngleInBetween");
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
                return -999;
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

    Manager::FunctionPtr ifNANgiveX(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        double defaultOutput;
        try {
          defaultOutput = Belle2::convertString<double>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The arguments of daughterInvM meta function must be a number!");
          return nullptr;
        }
        auto func = [var, defaultOutput](const Particle * particle) -> double {
          double output = var->function(particle);
          if (std::isnan(output)) return defaultOutput;
          else return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function ifNANgiveX");
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

    Manager::FunctionPtr max(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        const Variable::Manager::Var* var1 = Manager::Instance().getVariable(arguments[0]);
        const Variable::Manager::Var* var2 = Manager::Instance().getVariable(arguments[1]);

        if (!var1 or !var2)
          B2FATAL("One or both of the used variables doesn't exist!");

        auto func = [var1, var2](const Particle * particle) -> double {
          double max = var1->function(particle);
          if (max < var2->function(particle))
            max = var2->function(particle);
          return max;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function max");
      }
    }

    Manager::FunctionPtr min(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        const Variable::Manager::Var* var1 = Manager::Instance().getVariable(arguments[0]);
        const Variable::Manager::Var* var2 = Manager::Instance().getVariable(arguments[1]);

        if (!var1 or !var2)
          B2FATAL("One or both of the used variables doesn't exist!");

        auto func = [var1, var2](const Particle * particle) -> double {
          double min = var1->function(particle);
          if (min < var2->function(particle))
            min = var2->function(particle);
          return min;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function min");
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

    Manager::FunctionPtr mcDaughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of mcDaughter meta function must be integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (particle->getMCParticle()) // has MC match or is MCParticle
          {
            if (daughterNumber >= int(particle->getMCParticle()->getNDaughters())) {
              return -999;
            }
            Particle tempParticle = Particle(particle->getMCParticle()->getDaughters().at(daughterNumber));
            return var->function(&tempParticle);
          } else {
            return -999;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function mcDaughter");
      }
    }

    Manager::FunctionPtr mcMother(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999;
          if (particle->getMCParticle()) // has MC match or is MCParticle
          {
            if (particle->getMCParticle()->getMother() == nullptr) {
              return -999;
            }
            Particle tempParticle = Particle(particle->getMCParticle()->getMother());
            return var->function(&tempParticle);
          } else {
            return -999;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function mcMother");
      }
    }

    Manager::FunctionPtr genParticle(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int particleNumber = 0;
        try {
          particleNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of genParticle meta function must be integer!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

        auto func = [var, particleNumber](const Particle*) -> double {
          StoreArray<MCParticle> mcParticles("MCParticles");
          if (particleNumber >= mcParticles.getEntries())
          {
            return -999;
          }

          MCParticle* mcParticle = mcParticles[particleNumber];
          Particle part = Particle(mcParticle);
          return var->function(&part);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function genParticle");
      }
    }

    Manager::FunctionPtr genUpsilon4S(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        auto func = [var](const Particle*) -> double {
          StoreArray<MCParticle> mcParticles("MCParticles");
          if (mcParticles.getEntries() == 0)
          {
            return -999;
          }

          MCParticle* mcUpsilon4S = mcParticles[0];
          if (mcUpsilon4S->getPDG() != 300553)
          {
            return -999;
          }

          Particle upsilon4S = Particle(mcUpsilon4S);
          return var->function(&upsilon4S);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function genUpsilon4S");
      }
    }

    Manager::FunctionPtr getVariableByRank(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 4) {
        std::string listName = arguments[0];
        std::string rankedVariableName = arguments[1];
        std::string returnVariableName = arguments[2];
        std::string extraInfoName = rankedVariableName + "_rank";
        // 'rank' is correctly scoped, but cppcheck support the lambda
        // function syntax and throws a (wrong) variableScope error

        // cppcheck-suppress variableScope
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

    Manager::FunctionPtr matchedMC(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          const MCParticle* mcp = particle->getMCParticle();
          if (!mcp)   // Has no MC match and is no MCParticle
          {
            return -999;
          }
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

    Manager::FunctionPtr totalEnergyOfParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle * particle) -> double {

          (void) particle;
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to totalEnergyOfParticlesInList");
          double totalEnergy = 0;
          int nParticles = listOfParticles->getListSize();
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            const auto& frame = ReferenceFrame::GetCurrent();
            totalEnergy += frame.getMomentum(part).E();
          }
          return totalEnergy;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function totalEnergyOfParticlesInList");
      }
    }

    Manager::FunctionPtr totalPxOfParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to totalPxOfParticlesInList");
          double totalPx = 0;
          int nParticles = listOfParticles->getListSize();
          const auto& frame = ReferenceFrame::GetCurrent();
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            totalPx += frame.getMomentum(part).Px();
          }
          return totalPx;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function totalPxOfParticlesInList");
      }
    }

    Manager::FunctionPtr totalPyOfParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to totalPyOfParticlesInList");
          double totalPy = 0;
          int nParticles = listOfParticles->getListSize();
          const auto& frame = ReferenceFrame::GetCurrent();
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            totalPy += frame.getMomentum(part).Py();
          }
          return totalPy;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function totalPyOfParticlesInList");
      }
    }

    Manager::FunctionPtr totalPzOfParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to totalPzOfParticlesInList");
          double totalPz = 0;
          int nParticles = listOfParticles->getListSize();
          const auto& frame = ReferenceFrame::GetCurrent();
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            totalPz += frame.getMomentum(part).Pz();
          }
          return totalPz;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function totalPzOfParticlesInList");
      }
    }

    Manager::FunctionPtr invMassInLists(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {

        auto func = [arguments](const Particle * particle) -> double {

          TLorentzVector total4Vector;
          // To make sure particles in particlesList don't overlap.
          std::vector<Particle*> particlePool;

          (void) particle;
          for (unsigned int arg = 0; arg < arguments.size(); ++arg)
          {
            StoreObjPtr <ParticleList> listOfParticles(arguments[arg]);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << arguments[arg] << " given to invMassInLists");
            int nParticles = listOfParticles->getListSize();
            for (int i = 0; i < nParticles; i++) {
              bool overlaps = false;
              Particle* part = listOfParticles->getParticle(i);
              for (unsigned int j = 0; j < particlePool.size(); ++j) {
                Particle* poolPart = particlePool.at(j);
                if (part->overlapsWith(poolPart)) {
                  overlaps = true;
                  break;
                }
              }
              if (!overlaps) {
                total4Vector += part->get4Vector();
                particlePool.push_back(part);
              }
            }
          }
          double invariantMass = total4Vector.M();
          return invariantMass;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function invMassInLists");
      }
    }

    Manager::FunctionPtr totalECLEnergyOfParticlesInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle * particle) -> double {

          (void) particle;
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to totalEnergyOfParticlesInList");
          double totalEnergy = 0;
          int nParticles = listOfParticles->getListSize();
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            const ECLCluster* cluster = part->getECLCluster();
            if (cluster != nullptr) {
              totalEnergy += cluster->getEnergy();
            }
          }
          return totalEnergy;

        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function totalECLEnergyOfParticlesInList");
      }
    }

    Manager::FunctionPtr maxPtInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to maxPtInList");
          int nParticles = listOfParticles->getListSize();
          const auto& frame = ReferenceFrame::GetCurrent();
          double maxPt = 0;
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            const double Pt = frame.getMomentum(part).Pt();
            if (Pt > maxPt) maxPt = Pt;
          }
          return maxPt;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function maxPtInList");
      }
    }


    VARIABLE_GROUP("MetaFunctions");
    REGISTER_VARIABLE("nCleanedECLClusters(cut)", nCleanedECLClusters,
                      "[Eventbased] Returns the number of clean Clusters in the event\n"
                      "Clean clusters are defined by the clusters which pass the given cut assuming a photon hypothesis.");
    REGISTER_VARIABLE("nCleanedTracks(cut)", nCleanedTracks,
                      "[Eventbased] Returns the number of clean Tracks in the event\n"
                      "Clean tracks are defined by the tracks which pass the given cut assuming a pion hypothesis.");
    REGISTER_VARIABLE("formula(v1 + v2 * [v3 - v4] / v5^v6)", formula, R"DOCSTRING(
Returns the result of the given formula, where v1 to vN are variables or floating
point numbers. Currently the only supported operations are addtion (``+``),
subtraction (``-``), multiplication (``*``), division (``/``) and power (``^``
or ``**``). Parenthesis can be in the form of square brackets ``[v1 * v2]``
or normal brackets ``(v1 * v2)``. It will work also with variables taking
arguments. Operator precedence is taken into account. For example ::

    (daughter(0, E) + daughter(1, E))**2 - p**2 + 0.138

.. versionchanged:: release-03-00-00
   now both, ``[]`` and ``()`` can be used for grouping operations, ``**`` can
   be used for exponent and float literals are possible directly in the
   formula.
)DOCSTRING");
    REGISTER_VARIABLE("useRestFrame(variable)", useRestFrame,
                      "Returns the value of the variable using the rest frame of the given particle as current reference frame.\n"
                      "E.g. useRestFrame(daughter(0, p)) returns the total momentum of the first daughter in its mother's rest-frame");
    REGISTER_VARIABLE("useCMSFrame(variable)", useCMSFrame,
                      "Returns the value of the variable using the CMS frame as current reference frame.\n"
                      "E.g. useCMSFrame(E) returns the energy of a particle in the CMS frame.");
    REGISTER_VARIABLE("useLabFrame(variable)", useLabFrame,
                      "Returns the value of the variable using the lab frame as current reference frame.\n"
                      "The lab frame is the default reference frame, usually you don't need to use this meta-variable. E.g.\n"
                      "  - useLabFrame(E) returns the energy of a particle in the Lab frame, same as just E.\n"
                      "  - useRestFrame(daughter(0, formula(E - useLabFrame(E)))) only corner-cases like this need to use this variable.\n\n");
    REGISTER_VARIABLE("useROERecoilFrame(variable)", useROERecoilFrame,
                      "Returns the value of the variable using the rest frame of the ROE recoil as current reference frame.\n"
                      "E.g. useROERecoilFrame(E) returns the energy of a particle in the ROE recoil frame.");
    REGISTER_VARIABLE("passesCut(cut)", passesCut,
                      "Returns 1 if particle passes the cut otherwise 0.\n"
                      "Useful if you want to write out if a particle would have passed a cut or not.\n"
                      "Returns -999 if particle is a nullptr.");
    REGISTER_VARIABLE("passesEventCut(cut)", passesEventCut,
                      "[Eventbased] Returns 1 if event passes the cut otherwise 0.\n"
                      "Useful if you want to select events passing a cut without looping into particles, such as for skimming.\n");
    REGISTER_VARIABLE("countDaughters(cut)", countDaughters,
                      "Returns number of direct daughters which satisfy the cut.\n"
                      "Used by the skimming package (for what exactly?)\n"
                      "Returns -999 if particle is a nullptr.");
    REGISTER_VARIABLE("varFor(pdgCode, variable)", varFor,
                      "Returns the value of the variable for the given particle if its abs(pdgCode) agrees with the given one.\n"
                      "E.g. varFor(11, p) returns the momentum if the particle is an electron or a positron.");
    REGISTER_VARIABLE("nParticlesInList(particleListName)", nParticlesInList,
                      "Returns number of particles in the given particle List.");
    REGISTER_VARIABLE("isInList(particleListName)", isInList,
                      "Returns 1.0 if the particle is in the list provided, 0.0 if not. Note that this only checks the particle given. For daughters of composite particles, please see isDaughterOfList().");
    REGISTER_VARIABLE("isDaughterOfList(particleListNames)", isDaughterOfList,
                      "Returns 1 if the given particle is a daughter of at least one of the particles in the given particle Lists.");
    REGISTER_VARIABLE("isGrandDaughterOfList(particleListNames)", isGrandDaughterOfList,
                      "Returns 1 if the given particle is a grand daughter of at least one of the particles in the given particle Lists.");
    REGISTER_VARIABLE("daughter(i, variable)", daughter,
                      "Returns value of variable for the i-th daughter. E.g.\n"
                      "  - daughter(0, p) returns the total momentum of the first daughter.\n"
                      "  - daughter(0, daughter(1, p) returns the total momentum of the second daughter of the first daughter.\n\n"
                      "Returns -999 if particle is nullptr or if the given daughter-index is out of bound (>= amount of daughters).");
    REGISTER_VARIABLE("mcDaughter(i, variable)", mcDaughter,
                      "Returns the value of the requested variable for the i-th Monte Carlo daughter of the particle.\n"
                      "Returns -999 if the particle is nullptr, if the particle is not matched to an MC particle,"
                      "or if the i-th MC daughter does not exist.\n"
                      "E.g. mcDaughter(0, PDG) will return the PDG code of the first MC daughter of the matched MC"
                      "particle of the reconstructed particle the function is applied to./n"
                      "The meta variable can also be nested: mcDaughter(0, mcDaughter(1, PDG)).")
    REGISTER_VARIABLE("mcMother(variable)", mcMother,
                      "Returns the value of the requested variable for the Monte Carlo mother of the particle.\n"
                      "Returns -999 if the particle is nullptr, if the particle is not matched to an MC particle,"
                      "or if the MC mother does not exist.\n"
                      "E.g. mcMother(PDG) will return the PDG code of the MC mother of the matched MC"
                      "particle of the reconstructed particle the function is applied to.\n"
                      "The meta variable can also be nested: mcMother(mcMother(PDG)).")
    REGISTER_VARIABLE("genParticle(i, variable)", genParticle,
                      "[Eventbased] Returns function which returns the variable for the ith generator particle.\n"
                      "The arguments of the function must be:\n"
                      "    argument 1: Index of the particle in the MCParticle Array\n"
                      "    argument 2: Valid basf2 variable name of the function that shall be evaluated.\n"
                      "If the provided index goes beyond the length of the mcParticles array, -999 will be returned."
                      "E.g. genParticle(0, p) returns the total momentum of the first MC Particle, which is "
                      "the Upsilon(4S) in a generic decay.\n"
                      "     genParticle(0, mcDaughter(1, p) returns the total momentum of the second daughter of "
                      "the first MC Particle, which is the momentum of the second B meson in a generic decay.");
    REGISTER_VARIABLE("genUpsilon4S(variable)", genUpsilon4S,
                      "[Eventbased] Returns function which returns the variable evaluated for the generator level"
                      "Upsilon(4S).\n"
                      "The argument of the function must be a valid basf2 variable name of the function "
                      "that shall be evaluated.\n"
                      "If no generator level Upsilon(4S) exists for the event, -999 will be returned.\n"
                      "E.g. genUpsilon4S(p) returns the total momentum of the Upsilon(4S) in a generic decay.\n"
                      "     genUpsilon4S(mcDaughter(1, p) returns the total momentum of the second daughter of "
                      "the generator level Upsilon(4S), which is the momentum of the second B meson in a generic decay.");
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
                      "E.g. useRestFrame(daughterDiffOf(0, 1, p)) returns the momentum difference between first and second daughter in the rest frame of the given particle.\n"
                      "(That means that it returns p_j - p_i)\n"
                      "Nota Bene: for the particular case 'variable=phi' you should use the 'daughterDiffOfPhi' function.");
    REGISTER_VARIABLE("daughterDiffOfPhi(i, j)", daughterDiffOfPhi,
                      "Returns the difference in phi between the two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns phi_j - phi_i.\n"
                      "For a generic variable difference, see daughterDiffOf.");
    REGISTER_VARIABLE("daughterDiffOfClusterPhi(i, j)", daughterDiffOfClusterPhi,
                      "Returns the difference in phi between the ECLClusters of two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns phi_j - phi_i.\n"
                      "The function returns NaN if at least one of the daughters is not matched to or not based on an ECLCluster.\n"
                      "For a generic variable difference, see daughterDiffOf.");
    REGISTER_VARIABLE("daughterDiffOfPhiCMS(i, j)", daughterDiffOfPhiCMS,
                      "Returns the difference in phi between the two given daughters in the CMS frame.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns phi_j - phi_i.\n"
                      "For a generic variable difference, see daughterDiffOf.");
    REGISTER_VARIABLE("daughterDiffOfClusterPhiCMS(i, j)", daughterDiffOfClusterPhiCMS,
                      "Returns the difference in phi between the ECLClusters of two given daughters in the CMS frame.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns phi_j - phi_i.\n"
                      "The function returns NaN if at least one of the daughters is not matched to or not based on an ECLCluster.\n"
                      "For a generic variable difference, see daughterDiffOf.");
    REGISTER_VARIABLE("daughterNormDiffOf(i, j, variable)", daughterNormDiffOf,
                      "Returns the normalized difference of a variable between the two given daughters.\n"
                      "E.g. daughterNormDiffOf(0, 1, p) returns the normalized momentum difference between first and second daughter in the lab frame.");
    REGISTER_VARIABLE("daughterMotherDiffOf(i, variable)", daughterMotherDiffOf,
                      "Returns the difference of a variable between the given daughter and the mother particle itself.\n"
                      "E.g. useRestFrame(daughterMotherDiffOf(0, p)) returns the momentum difference between the given particle and its first daughter in the rest frame of the mother.");
    REGISTER_VARIABLE("daughterMotherNormDiffOf(i, variable)", daughterMotherNormDiffOf,
                      "Returns the normalized difference of a variable between the given daughter and the mother particle itself.\n"
                      "E.g. daughterMotherNormDiffOf(1, p) returns the normalized momentum difference between the given particle and its second daughter in the lab frame.");
    REGISTER_VARIABLE("daughterAngleInBetween(i, j)", daughterAngleInBetween,
                      "If two indices given: Variable returns the angle between the momenta of the two given daughters.\n"
                      "If three indices given: Variable returns the angle between the momentum of the third particle and a vector "
                      "which is the sum of the first two daughter momenta.\n"
                      "E.g. useLabFrame(daughterAngleInBetween(0, 1)) returns the angle between first and second daughter in the Lab frame.");
    REGISTER_VARIABLE("daughterClusterAngleInBetween(i, j)", daughterClusterAngleInBetween,
                      "Returns function which returns the angle between clusters associated to the two daughters."
                      "If two indices given: returns the angle between the momenta of the clusters associated to the two given daughters."
                      "If three indices given: returns the angle between the momentum of the third particle's cluster and a vector "
                      "which is the sum of the first two daughter's cluster momenta."
                      "Returns nan if any of the daughters specified don't have an associated cluster."
                      "The arguments in the argument vector must be integers corresponding to the ith and jth (and kth) daughters.");
    REGISTER_VARIABLE("daughterInvM(i, j)", daughterInvM,
                      "Returns the invariant Mass adding the Lorentz vectors of the given daughters.\n"
                      "E.g. daughterInvM(0, 1, 2) returns the invariant Mass m = sqrt((p0 + p1 + p2)^2) of first, second and third daughter.");
    REGISTER_VARIABLE("extraInfo(name)", extraInfo,
                      "Returns extra info stored under the given name.\n"
                      "The extraInfo has to be set first by a module like MVAExpert. If nothing is set under this name, -999 is returned.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the MVAExpert.");
    REGISTER_VARIABLE("eventExtraInfo(name)", eventExtraInfo,
                      "[Eventbased] Returns extra info stored under the given name in the event extra info.\n"
                      "The extraInfo has to be set first by another module like MVAExpert in event mode.\n"
                      "E.g. extraInfo(SignalProbability) returns the SignalProbability calculated by the MVAExpert for an event.");
    REGISTER_VARIABLE("eventCached(variable)", eventCached,
                      "[Eventbased] Returns value of event-based variable and caches this value in the EventExtraInfo.\n"
                      "The result of second call to this variable in the same event will be provided from the cache.");
    REGISTER_VARIABLE("particleCached(variable)", particleCached,
                      "Returns value of given variable and caches this value in the ParticleExtraInfo of the provided particle.\n"
                      "The result of second call to this variable on the same particle will be provided from the cache.");
    REGISTER_VARIABLE("modulo(variable, n)", modulo,
                      "Returns rest of division of variable by n.");
    REGISTER_VARIABLE("abs(variable)", abs,
                      "Returns absolute value of the given variable.\n"
                      "E.g. abs(mcPDG) returns the absolute value of the mcPDG, which is often useful for cuts.");
    REGISTER_VARIABLE("max(var1,var2)", max,
                      "Returns max value of two variables.\n");
    REGISTER_VARIABLE("min(var1,var2)", max,
                      "Returns min value of two variables.\n");
    REGISTER_VARIABLE("sin(variable)", sin,
                      "Returns sin value of the given variable.\n"
                      "E.g. sin(?) returns the sine of the value of the variable.");
    REGISTER_VARIABLE("cos(variable)", cos,
                      "Returns cos value of the given variable.\n"
                      "E.g. sin(?) returns the cosine of the value of the variable.");
    REGISTER_VARIABLE("isNAN(variable)", isNAN,
                      "Returns true if variable value evaluates to nan (determined via std::isnan(double)).\n"
                      "Useful for debugging.");
    REGISTER_VARIABLE("ifNANgiveX(variable, x)", ifNANgiveX,
                      "Returns x (has to be a number) if variable value is nan (determined via std::isnan(double)).\n"
                      "Useful for technical purposes while training MVAs.");
    REGISTER_VARIABLE("isInfinity(variable)", isInfinity,
                      "Returns true if variable value evaluates to infinity (determined via std::isinf(double)).\n"
                      "Useful for debugging.");
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
    REGISTER_VARIABLE("totalEnergyOfParticlesInList(particleListName)", totalEnergyOfParticlesInList,
                      "Returns the total energy of particles in the given particle List.");
    REGISTER_VARIABLE("totalPxOfParticlesInList(particleListName)", totalPxOfParticlesInList,
                      "Returns the total momentum Px of particles in the given particle List.");
    REGISTER_VARIABLE("totalPyOfParticlesInList(particleListName)", totalPyOfParticlesInList,
                      "Returns the total momentum Py of particles in the given particle List.");
    REGISTER_VARIABLE("totalPzOfParticlesInList(particleListName)", totalPzOfParticlesInList,
                      "Returns the total momentum Pz of particles in the given particle List.");
    REGISTER_VARIABLE("invMassInLists(pList1, pList2, ...)", invMassInLists,
                      "Returns the invariant mass of the combination of particles in the given particle lists.");
    REGISTER_VARIABLE("totalECLEnergyOfParticlesInList(particleListName)", totalECLEnergyOfParticlesInList,
                      "Returns the total ECL energy of particles in the given particle List.");
    REGISTER_VARIABLE("maxPtInList(particleListName)", maxPtInList,
                      "Returns maximum transverse momentum Pt in the given particle List.");
  }
}
