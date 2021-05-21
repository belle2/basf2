/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2020 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Anze Zupanc, Sam Cunliffe,                  *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/MetaVariables.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
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
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <boost/algorithm/string.hpp>
#include <limits>

#include <cmath>
#include <stdexcept>
#include <memory>
#include <string>
#include <regex>

#include <TDatabasePDG.h>

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
        B2FATAL("Wrong number of arguments for meta function useRestFrame");
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
        B2FATAL("Wrong number of arguments for meta function useCMSFrame");
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
        B2FATAL("Wrong number of arguments for meta function useLabFrame");
      }
    }

    Manager::FunctionPtr useTagSideRecoilRestFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        int daughterIndexTagB = 0;
        try {
          daughterIndexTagB = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2FATAL("Second argument of useTagSideRecoilRestFrame meta function must be integer!");
        }

        auto func = [var, daughterIndexTagB](const Particle * particle) -> double {
          if (particle->getPDGCode() != 300553)
          {
            B2ERROR("Variable should only be used on a Upsilon(4S) Particle List!");
            return std::numeric_limits<float>::quiet_NaN();
          }

          PCmsLabTransform T;
          TLorentzVector pSigB = T.getBeamFourMomentum() - particle->getDaughter(daughterIndexTagB)->get4Vector();
          Particle tmp(pSigB, -particle->getDaughter(daughterIndexTagB)->getPDGCode());

          UseReferenceFrame<RestFrame> frame(&tmp);
          double result = var->function(particle);
          return result;
        };

        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function useTagSideRecoilRestFrame");
      }
    }



    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            B2WARNING("Returns NaN because the particle is nullptr! If you want EventExtraInfo variables, please use eventExtraInfo() instead");
            return std::numeric_limits<float>::quiet_NaN();
          }
          if (particle->hasExtraInfo(extraInfoName))
          {
            return particle->getExtraInfo(extraInfoName);
          } else {
            return std::numeric_limits<float>::quiet_NaN();
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function extraInfo");
      }
    }

    Manager::FunctionPtr eventExtraInfo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName](const Particle*) -> double {
          StoreObjPtr<EventExtraInfo> eventExtraInfo;
          if (not eventExtraInfo.isValid())
            return std::numeric_limits<float>::quiet_NaN();
          if (eventExtraInfo->hasExtraInfo(extraInfoName))
          {
            return eventExtraInfo->getExtraInfo(extraInfoName);
          } else {
            return std::numeric_limits<float>::quiet_NaN();
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function extraInfo");
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
        B2FATAL("Wrong number of arguments for meta function eventCached");
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
        B2FATAL("Wrong number of arguments for meta function particleCached");
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
            if (!cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
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
            return std::numeric_limits<float>::quiet_NaN();
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
        } catch (std::invalid_argument&) {
          B2FATAL("The first argument of varFor meta function must be a positive integer!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [pdgCode, var](const Particle * particle) -> double {

          if (std::abs(particle -> getPDGCode()) == std::abs(pdgCode))
            return var -> function(particle);
          else return std::numeric_limits<float>::quiet_NaN();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function varFor");
      }
    }

    Manager::FunctionPtr varForMCGen(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {

          if (particle -> getMCParticle())
          {
            if (particle -> getMCParticle() -> getStatus(MCParticle::c_PrimaryParticle)
            && (! particle -> getMCParticle() -> getStatus(MCParticle::c_IsVirtual))
            && (! particle -> getMCParticle() -> getStatus(MCParticle::c_Initial))) {
              return var -> function(particle);
            } else return std::numeric_limits<float>::quiet_NaN();
          } else return std::numeric_limits<float>::quiet_NaN();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function varForMCGen");
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

    Manager::FunctionPtr sourceObjectIsInList(const std::vector<std::string>& arguments)
    {
      // unpack arguments, there should be only one: the name of the list we're checking
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments for sourceObjectIsInList");
      }
      auto listName = arguments[0];

      auto func = [listName](const Particle * particle) -> double {

        // check the list exists
        StoreObjPtr<ParticleList> list(listName);
        if (!(list.isValid()))
        {
          B2FATAL("Invalid Listname " << listName << " given to sourceObjectIsInList");
        }

        // this only makes sense for particles that are *not* composite and come
        // from some mdst object (tracks, clusters..)
        Particle::EParticleSourceObject particlesource = particle->getParticleSource();
        if (particlesource == Particle::EParticleSourceObject::c_Composite
        or particlesource == Particle::EParticleSourceObject::c_Undefined)
          return -1.0;

        // it *is* possible to have a particle list from different sources (like
        // hadrons from the ECL and KLM) so we have to check each particle in
        // the list individually
        for (unsigned i = 0; i < list->getListSize(); ++i)
        {
          Particle* iparticle = list->getParticle(i);
          if (particlesource == iparticle->getParticleSource())
            if (particle->getMdstArrayIndex() == iparticle->getMdstArrayIndex())
              return 1.0;
        }
        return 0.0;

      };
      return func;
    }

    Manager::FunctionPtr mcParticleIsInMCList(const std::vector<std::string>& arguments)
    {
      // unpack arguments, there should be only one: the name of the list we're checking
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments for mcParticleIsInMCList");
      }
      auto listName = arguments[0];

      auto func = [listName](const Particle * particle) -> double {

        // check the list exists
        StoreObjPtr<ParticleList> list(listName);
        if (!(list.isValid()))
          B2FATAL("Invalid Listname " << listName << " given to mcParticleIsInMCList");

        // this can only be true for mc-matched particles or particles are created from MCParticles
        const MCParticle* mcp = particle->getMCParticle();
        if (mcp == nullptr) return 0.0;

        // check every particle in the input list is not matched to (or created from) the same MCParticle
        for (unsigned i = 0; i < list->getListSize(); ++i)
        {
          const MCParticle* imcp = list->getParticle(i)->getMCParticle();
          if ((imcp != nullptr) and (mcp->getArrayIndex() == imcp->getArrayIndex()))
            return 1.0;
        }
        return 0.0;
      };
      return func;
    }

    Manager::FunctionPtr isDaughterOfList(const std::vector<std::string>& arguments)
    {
      B2WARNING("isDaughterOfList is outdated and replaced by isDescendantOfList.");
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("1"));
      return isDescendantOfList(new_arguments);
    }

    Manager::FunctionPtr isGrandDaughterOfList(const std::vector<std::string>& arguments)
    {
      B2WARNING("isGrandDaughterOfList is outdated and replaced by isDescendantOfList.");
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("2"));
      return isDescendantOfList(new_arguments);
    }

    Manager::FunctionPtr isDescendantOfList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {
        auto listNames = arguments;
        auto func = [listNames](const Particle * particle) -> double {
          double output = 0;
          int generation_flag = -1;
          try {
            generation_flag = Belle2::convertString<int>(listNames.back());
          } catch (std::exception& e) {}


          for (auto& iListName : listNames)
          {
            try {
              Belle2::convertString<int>(iListName);
              continue;
            } catch (std::exception& e) {}

            // Creating recursive lambda
            auto list_comparison  = [](auto && self, const Particle * m, const Particle * p, int flag)-> int {
              int result = 0;
              for (unsigned i = 0; i < m->getNDaughters(); ++i)
              {
                const Particle* daughter = m->getDaughter(i);
                if ((flag == 1.) or (flag < 0)) {
                  if (p->isCopyOf(daughter)) {
                    return 1;
                  }
                }

                if (flag != 1.) {
                  if (daughter->getNDaughters() > 0) {
                    result = self(self, daughter, p, flag - 1);
                    if (result == 1) {
                      return 1;
                    }
                  }
                }
              }
              return result;
            };


            StoreObjPtr<ParticleList> listOfParticles(iListName);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << iListName << " given to isDescendantOfList");

            for (unsigned i = 0; i < listOfParticles->getListSize(); ++i) {
              Particle* iParticle = listOfParticles->getParticle(i);
              output = list_comparison(list_comparison, iParticle, particle, generation_flag);
              if (output == 1) {
                return output;
              }
            }
          }
          return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isDescendantOfList");
      }
    }

    Manager::FunctionPtr isMCDescendantOfList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {
        auto listNames = arguments;
        auto func = [listNames](const Particle * particle) -> double {
          double output = 0;
          int generation_flag = -1;
          try {
            generation_flag = Belle2::convertString<int>(listNames.back());
          } catch (std::exception& e) {}

          if (particle->getMCParticle() == nullptr)
          {
            return 0;
          }

          for (auto& iListName : listNames)
          {
            try {
              std::stod(iListName);
              continue;
            } catch (std::exception& e) {}
            // Creating recursive lambda
            auto list_comparison  = [](auto && self, const Particle * m, const Particle * p, int flag)-> int {
              int result = 0;
              for (unsigned i = 0; i < m->getNDaughters(); ++i)
              {
                const Particle* daughter = m->getDaughter(i);
                if ((flag == 1.) or (flag < 0)) {
                  if (daughter->getMCParticle() != nullptr) {
                    if (p->getMCParticle()->getArrayIndex() == daughter->getMCParticle()->getArrayIndex()) {
                      return 1;
                    }
                  }
                }
                if (flag != 1.) {
                  if (daughter->getNDaughters() > 0) {
                    result = self(self, daughter, p, flag - 1);
                    if (result == 1) {
                      return 1;
                    }
                  }
                }
              }
              return result;
            };


            StoreObjPtr<ParticleList> listOfParticles(iListName);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << iListName << " given to isMCDescendantOfList");

            for (unsigned i = 0; i < listOfParticles->getListSize(); ++i) {
              Particle* iParticle = listOfParticles->getParticle(i);
              output = list_comparison(list_comparison, iParticle, particle, generation_flag);
              if (output == 1) {
                return output;
              }
            }
          }
          return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isMCDescendantOfList");
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
          double min = std::numeric_limits<double>::quiet_NaN();
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            double iValue = var->function(particle->getDaughter(j));
            if (std::isnan(iValue)) continue;
            if (std::isnan(min)) min = iValue;
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
          double max = std::numeric_limits<double>::quiet_NaN();
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            double iValue = var->function(particle->getDaughter(j));
            if (std::isnan(iValue)) continue;
            if (std::isnan(max)) max = iValue;
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
        } catch (std::invalid_argument&) {
          B2FATAL("First two arguments of daughterDiffOf meta function must be integers!");
        }
        auto variablename = arguments[2];
        auto func = [variablename, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          else {
            const Variable::Manager::Var* var = Manager::Instance().getVariable(variablename);
            double diff = var->function(particle->getDaughter(jDaughterNumber)) - var->function(particle->getDaughter(iDaughterNumber));
            if (variablename == "phi" or variablename == "clusterPhi" or std::regex_match(variablename, std::regex("use.*Frame\\(phi\\)")) or std::regex_match(variablename, std::regex("use.*Frame\\(clusterPhi\\)")))
            {
              if (fabs(diff) > M_PI) {
                if (diff > M_PI) {
                  diff = diff - 2 * M_PI;
                } else {
                  diff = 2 * M_PI + diff;
                }
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterDiffOf");
      }
    }

    Manager::FunctionPtr mcDaughterDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2FATAL("First two arguments of mcDaughterDiffOf meta function must be integers!");
        }
        auto variablename = arguments[2];
        auto func = [variablename, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          if (particle->getDaughter(jDaughterNumber)->getMCParticle() == nullptr || particle->getDaughter(iDaughterNumber)->getMCParticle() == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          else {
            const MCParticle* iMcDaughter = particle->getDaughter(iDaughterNumber)->getMCParticle();
            const MCParticle* jMcDaughter = particle->getDaughter(jDaughterNumber)->getMCParticle();
            Particle iTmpPart(iMcDaughter);
            Particle jTmpPart(jMcDaughter);
            const Variable::Manager::Var* var = Manager::Instance().getVariable(variablename);
            double diff = var->function(&jTmpPart) - var->function(&iTmpPart);
            if (variablename == "phi" or std::regex_match(variablename, std::regex("use.*Frame\\(phi\\)")))
            {
              if (fabs(diff) > M_PI) {
                if (diff > M_PI) {
                  diff = diff - 2 * M_PI;
                } else {
                  diff = 2 * M_PI + diff;
                }
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function mcDaughterDiffOf");
      }
    }

    Manager::FunctionPtr grandDaughterDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 5) {
        int iDaughterNumber = 0, jDaughterNumber = 0, agrandDaughterNumber = 0, bgrandDaughterNumber = 0;
        try {
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
          agrandDaughterNumber = Belle2::convertString<int>(arguments[2]);
          bgrandDaughterNumber = Belle2::convertString<int>(arguments[3]);
        } catch (std::invalid_argument&) {
          B2FATAL("First four arguments of grandDaughterDiffOf meta function must be integers!");
        }
        auto variablename = arguments[4];
        auto func = [variablename, iDaughterNumber, jDaughterNumber, agrandDaughterNumber,
        bgrandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          if (agrandDaughterNumber >= int((particle->getDaughter(iDaughterNumber))->getNDaughters()) || bgrandDaughterNumber >= int((particle->getDaughter(jDaughterNumber))->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          else {
            const Variable::Manager::Var* var = Manager::Instance().getVariable(variablename);
            double diff = var->function((particle->getDaughter(jDaughterNumber))->getDaughter(bgrandDaughterNumber)) - var->function((particle->getDaughter(iDaughterNumber))->getDaughter(agrandDaughterNumber));
            if (variablename == "phi" or variablename == "clusterPhi" or std::regex_match(variablename, std::regex("use.*Frame\\(phi\\)")) or std::regex_match(variablename, std::regex("use.*Frame\\(clusterPhi\\)")))
            {
              if (fabs(diff) > M_PI) {
                if (diff > M_PI) {
                  diff = diff - 2 * M_PI;
                } else {
                  diff = 2 * M_PI + diff;
                }
              }
            }
            return diff;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function grandDaughterDiffOf");
      }
    }

    Manager::FunctionPtr daughterDiffOfPhi(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("phi"));
      return daughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr mcDaughterDiffOfPhi(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("phi"));
      return mcDaughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr grandDaughterDiffOfPhi(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("phi"));
      return grandDaughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr daughterDiffOfClusterPhi(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("clusterPhi"));
      return daughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr grandDaughterDiffOfClusterPhi(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("clusterPhi"));
      return grandDaughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr daughterDiffOfPhiCMS(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("useCMSFrame(phi)"));
      return daughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr mcDaughterDiffOfPhiCMS(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("useCMSFrame(phi)"));
      return mcDaughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr daughterDiffOfClusterPhiCMS(const std::vector<std::string>& arguments)
    {
      std::vector<std::string> new_arguments = arguments;
      new_arguments.push_back(std::string("useCMSFrame(clusterPhi)"));
      return daughterDiffOf(new_arguments);
    }

    Manager::FunctionPtr daughterNormDiffOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        int iDaughterNumber = 0;
        int jDaughterNumber = 0;
        try {
          iDaughterNumber = Belle2::convertString<int>(arguments[0]);
          jDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2FATAL("First two arguments of daughterDiffOf meta function must be integers!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[2]);
        auto func = [var, iDaughterNumber, jDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (iDaughterNumber >= int(particle->getNDaughters()) || jDaughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          else {
            double iValue = var->function(particle->getDaughter(iDaughterNumber));
            double jValue = var->function(particle->getDaughter(jDaughterNumber));
            return (jValue - iValue) / (jValue + iValue);
          }
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
        } catch (std::invalid_argument&) {
          B2FATAL("First argument of daughterMotherDiffOf meta function must be integer!");
        }
        auto variablename = arguments[1];
        auto func = [variablename, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (daughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          else {
            const Variable::Manager::Var* var = Manager::Instance().getVariable(variablename);
            double diff = var->function(particle) - var->function(particle->getDaughter(daughterNumber));
            if (variablename == "phi" or variablename == "useCMSFrame(phi)")
            {
              if (fabs(diff) > M_PI) {
                if (diff > M_PI) {
                  diff = diff - 2 * M_PI;
                } else {
                  diff = 2 * M_PI + diff;
                }
              }
            }
            return diff;
          }
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
        } catch (std::invalid_argument&) {
          B2FATAL("First argument of daughterMotherDiffOf meta function must be integer!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (daughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<double>::quiet_NaN();
          else {
            double daughterValue = var->function(particle->getDaughter(daughterNumber));
            double motherValue = var->function(particle);
            return (motherValue - daughterValue) / (motherValue + daughterValue);
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterMotherNormDiffOf");
      }
    }

    Manager::FunctionPtr daughterAngle(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 || arguments.size() == 3) {

        auto func = [arguments](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();

          std::vector<TLorentzVector> pDaus;
          const auto& frame = ReferenceFrame::GetCurrent();

          // Parses the generalized indexes and fetches the 4-momenta of the particles of interest
          for (auto& generalizedIndex : arguments)
          {
            const Particle* dauPart = particle->getParticleFromGeneralizedIndexString(generalizedIndex);
            if (dauPart)
              pDaus.push_back(frame.getMomentum(dauPart));
            else {
              B2WARNING("Trying to access a daughter that does not exist. Index = " << generalizedIndex);
              return std::numeric_limits<double>::quiet_NaN();
            }
          }

          // Calculates the angle between the selected particles
          if (pDaus.size() == 2)
            return pDaus[0].Vect().Angle(pDaus[1].Vect());
          else
            return pDaus[2].Vect().Angle(pDaus[0].Vect() + pDaus[1].Vect());
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterAngle");
      }
    }

    Manager::FunctionPtr grandDaughterDecayAngle(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {

        auto func = [arguments](const Particle * particle) -> double {
          if (!particle)
            return std::numeric_limits<double>::quiet_NaN();

          unsigned daughterIndex, grandDaughterIndex;
          try {
            daughterIndex = Belle2::convertString<int>(arguments[0]);
            grandDaughterIndex = Belle2::convertString<int>(arguments[1]);
          } catch (std::invalid_argument&)
          {
            B2FATAL("The arguments of grandDaughterDecayAngle have to be integers!");
          }

          if (daughterIndex >= particle->getNDaughters())
            return std::numeric_limits<float>::quiet_NaN();

          const Particle* daughter = particle->getDaughter(daughterIndex);

          if (grandDaughterIndex >= daughter->getNDaughters())
            return std::numeric_limits<float>::quiet_NaN();

          TVector3  boost = - (daughter->get4Vector().BoostVector());

          TLorentzVector motherMomentum = - particle->get4Vector();
          motherMomentum.Boost(boost);

          TLorentzVector grandDaughterMomentum = daughter->getDaughter(grandDaughterIndex)->get4Vector();
          grandDaughterMomentum.Boost(boost);

          return motherMomentum.Angle(grandDaughterMomentum.Vect());
        };
        return func;
      } else {
        B2FATAL("The meta variable grandDaughterDecayAngle needs exactly two integers as arguments!");
      }
    }

    Manager::FunctionPtr mcDaughterAngle(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 || arguments.size() == 3) {

        auto func = [arguments](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();

          std::vector<TLorentzVector> pDaus;
          const auto& frame = ReferenceFrame::GetCurrent();

          // Parses the generalized indexes and fetches the 4-momenta of the particles of interest
          for (auto& generalizedIndex : arguments)
          {
            const Particle* dauPart = particle->getParticleFromGeneralizedIndexString(generalizedIndex);
            if (dauPart == nullptr)
              return std::numeric_limits<double>::quiet_NaN();

            const MCParticle* dauMcPart = dauPart->getMCParticle();
            if (dauMcPart == nullptr)
              return std::numeric_limits<double>::quiet_NaN();

            pDaus.push_back(frame.getMomentum(dauMcPart->get4Vector()));
          }

          // Calculates the angle between the selected particles
          if (pDaus.size() == 2)
            return pDaus[0].Vect().Angle(pDaus[1].Vect());
          else
            return pDaus[2].Vect().Angle(pDaus[0].Vect() + pDaus[1].Vect());
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function mcDaughterAngle");
      }
    }

    Manager::FunctionPtr daughterClusterAngleInBetween(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2 || arguments.size() == 3) {
        std::vector<int> daughterIndices;
        try {
          for (auto& argument : arguments) daughterIndices.push_back(Belle2::convertString<int>(argument));
        } catch (std::invalid_argument&) {
          B2FATAL("The arguments of daughterClusterAngleInBetween meta function must be integers!");
        }
        auto func = [daughterIndices](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();
          if (daughterIndices.size() == 2)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters()))
              return std::numeric_limits<double>::quiet_NaN();
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              const ECLCluster* clusteri = (particle->getDaughter(daughterIndices[0]))->getECLCluster();
              const ECLCluster* clusterj = (particle->getDaughter(daughterIndices[1]))->getECLCluster();
              const ECLCluster::EHypothesisBit clusteriBit = (particle->getDaughter(daughterIndices[0]))->getECLClusterEHypothesisBit();
              const ECLCluster::EHypothesisBit clusterjBit = (particle->getDaughter(daughterIndices[1]))->getECLClusterEHypothesisBit();
              if (clusteri and clusterj) {
                ClusterUtils clusutils;
                TVector3 pi = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusteri, clusteriBit)).Vect();
                TVector3 pj = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterj, clusterjBit)).Vect();
                return pi.Angle(pj);
              }
              return std::numeric_limits<float>::quiet_NaN();
            }
          } else if (daughterIndices.size() == 3)
          {
            if (daughterIndices[0] >= int(particle->getNDaughters()) || daughterIndices[1] >= int(particle->getNDaughters())
                || daughterIndices[2] >= int(particle->getNDaughters())) return std::numeric_limits<double>::quiet_NaN();
            else {
              const auto& frame = ReferenceFrame::GetCurrent();
              const ECLCluster* clusteri = (particle->getDaughter(daughterIndices[0]))->getECLCluster();
              const ECLCluster* clusterj = (particle->getDaughter(daughterIndices[1]))->getECLCluster();
              const ECLCluster* clusterk = (particle->getDaughter(daughterIndices[2]))->getECLCluster();
              const ECLCluster::EHypothesisBit clusteriBit = (particle->getDaughter(daughterIndices[0]))->getECLClusterEHypothesisBit();
              const ECLCluster::EHypothesisBit clusterjBit = (particle->getDaughter(daughterIndices[1]))->getECLClusterEHypothesisBit();
              const ECLCluster::EHypothesisBit clusterkBit = (particle->getDaughter(daughterIndices[2]))->getECLClusterEHypothesisBit();

              if (clusteri and clusterj and clusterk) {
                ClusterUtils clusutils;
                TVector3 pi = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusteri, clusteriBit)).Vect();
                TVector3 pj = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterj, clusterjBit)).Vect();
                TVector3 pk = frame.getMomentum(clusutils.Get4MomentumFromCluster(clusterk, clusterkBit)).Vect();
                return pk.Angle(pi + pj);
              }
              return std::numeric_limits<float>::quiet_NaN();
            }
          } else return std::numeric_limits<double>::quiet_NaN();

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
        } catch (std::invalid_argument&) {
          B2FATAL("The arguments of daughterInvM meta function must be integers!");
        }
        auto func = [daughterIndices](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN();
          else {
            const auto& frame = ReferenceFrame::GetCurrent();
            TLorentzVector pSum;

            for (auto& index : daughterIndices)
            {
              if (index >= int(particle->getNDaughters())) {
                return std::numeric_limits<float>::quiet_NaN();
              } else pSum += frame.getMomentum(particle->getDaughter(index));
            }

            return pSum.M();
          }
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
        } catch (std::invalid_argument&) {
          B2FATAL("Second argument of modulo meta function must be integer!");
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
        } catch (std::invalid_argument&) {
          B2FATAL("The second argument of ifNANgiveX meta function must be a number!");
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

    Manager::FunctionPtr unmask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() >= 2) {
        // get the function pointer of variable to be unmasked
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        // get the final mask which summarize all the input masks
        int finalMask = 0;
        for (size_t i = 1; i < arguments.size(); ++i) {
          try {
            finalMask |= Belle2::convertString<int>(arguments[i]);
          } catch (std::invalid_argument&) {
            B2FATAL("The input flags to meta function unmask() should be integer!");
            return nullptr;
          }
        }

        // unmask the variable
        auto func = [var, finalMask](const Particle * particle) -> double {
          int value = int(var->function(particle));

          // judge if the value is nan before unmasking
          if (std::isnan(value))
            return std::numeric_limits<double>::quiet_NaN();

          // apply the final mask
          value &= (~finalMask);

          return value;
        };
        return func;

      } else {
        B2FATAL("Meta function unmask needs at least two arguments!");
      }
    }


    Manager::FunctionPtr conditionalVariableSelector(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {

        std::string cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));

        const Variable::Manager::Var* variableIfTrue = Manager::Instance().getVariable(arguments[1]);
        const Variable::Manager::Var* variableIfFalse = Manager::Instance().getVariable(arguments[2]);

        auto func = [cut, variableIfTrue, variableIfFalse](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN();
          if (cut->check(particle))
            return variableIfTrue->function(particle);
          else
            return variableIfFalse->function(particle);
        };
        return func;

      } else {
        B2FATAL("Wrong number of arguments for meta function conditionalVariableSelector");
      }
    }


    Manager::FunctionPtr pValueCombination(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0) {
        std::vector<const Variable::Manager::Var*> variables;
        for (auto& argument : arguments)
          variables.push_back(Manager::Instance().getVariable(argument));

        auto func = [variables, arguments](const Particle * particle) -> double {
          double pValueProduct = 1.;
          for (auto variable : variables)
          {
            double pValue = variable->function(particle);
            if (pValue < 0)
              return -1;
            else
              pValueProduct *= pValue;
          }
          double pValueSum = 1.;
          double factorial = 1.;
          for (unsigned int i = 1; i < arguments.size(); ++i)
          {
            factorial *= i;
            pValueSum += pow(-std::log(pValueProduct), i) / factorial;
          }
          return pValueProduct * pValueSum;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function pValueCombination");
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
          if (min > var2->function(particle))
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

    Manager::FunctionPtr asin(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::asin(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function asin");
      }
    }

    Manager::FunctionPtr cos(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::cos(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function cos");
      }
    }

    Manager::FunctionPtr acos(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::acos(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function acos");
      }
    }

    Manager::FunctionPtr exp(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::exp(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function exp");
      }
    }

    Manager::FunctionPtr log(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::log(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function log");
      }
    }

    Manager::FunctionPtr log10(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double { return std::log10(var->function(particle)); };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function log10");
      }
    }

    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2FATAL("First argument of daughter meta function must be integer!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN();
          if (daughterNumber >= int(particle->getNDaughters()))
            return std::numeric_limits<float>::quiet_NaN();
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
        } catch (std::invalid_argument&) {
          B2FATAL("First argument of mcDaughter meta function must be integer!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN();
          if (particle->getMCParticle()) // has MC match or is MCParticle
          {
            if (daughterNumber >= int(particle->getMCParticle()->getNDaughters())) {
              return std::numeric_limits<float>::quiet_NaN();
            }
            Particle tempParticle = Particle(particle->getMCParticle()->getDaughters().at(daughterNumber));
            return var->function(&tempParticle);
          } else {
            return std::numeric_limits<float>::quiet_NaN();
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
            return std::numeric_limits<float>::quiet_NaN();
          if (particle->getMCParticle()) // has MC match or is MCParticle
          {
            if (particle->getMCParticle()->getMother() == nullptr) {
              return std::numeric_limits<float>::quiet_NaN();
            }
            Particle tempParticle = Particle(particle->getMCParticle()->getMother());
            return var->function(&tempParticle);
          } else {
            return std::numeric_limits<float>::quiet_NaN();
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
        } catch (std::invalid_argument&) {
          B2FATAL("First argument of genParticle meta function must be integer!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

        auto func = [var, particleNumber](const Particle*) -> double {
          StoreArray<MCParticle> mcParticles("MCParticles");
          if (particleNumber >= mcParticles.getEntries())
          {
            return std::numeric_limits<float>::quiet_NaN();
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
            return std::numeric_limits<float>::quiet_NaN();
          }

          MCParticle* mcUpsilon4S = mcParticles[0];
          if (mcUpsilon4S->getPDG() != 300553)
          {
            return std::numeric_limits<float>::quiet_NaN();
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
        int rank = 1;
        try {
          rank = Belle2::convertString<int>(arguments[3]);
        } catch (std::invalid_argument&)  {
          B2ERROR("3rd argument of getVariableByRank meta function (Rank) must be an integer!");
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
        int pdgCode = Const::electron.getPDGCode();
        if (arguments.size() == 2) {
          B2INFO("Use pdgCode of electron as default in meta variable veto, other arguments: " << roeListName << ", " << cutString);
        } else {
          try {
            pdgCode = Belle2::convertString<int>(arguments[2]);;
          } catch (std::invalid_argument&) {
            B2FATAL("Third argument of veto meta function must be integer!");
          }
        }

        auto flavourType = (Belle2::EvtPDLUtil::hasAntiParticle(pdgCode)) ? Particle::c_Flavored : Particle::c_Unflavored;
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));

        auto func = [roeListName, cut, pdgCode, flavourType](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN();
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
            return std::numeric_limits<float>::quiet_NaN();
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
            return std::numeric_limits<float>::quiet_NaN();
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
        } catch (std::invalid_argument&) {
          B2FATAL("Argument must be an integer value.");
        }

        auto func = [inputPDG](const Particle * particle) -> double{
          const MCParticle* mcp = particle->getMCParticle();
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
          for (const auto& argument : arguments)
          {
            StoreObjPtr <ParticleList> listOfParticles(argument);

            if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << argument << " given to invMassInLists");
            int nParticles = listOfParticles->getListSize();
            for (int i = 0; i < nParticles; i++) {
              bool overlaps = false;
              Particle* part = listOfParticles->getParticle(i);
              for (auto poolPart : particlePool) {
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
            const ECLCluster::EHypothesisBit clusterHypothesis = part->getECLClusterEHypothesisBit();
            if (cluster != nullptr) {
              totalEnergy += cluster->getEnergy(clusterHypothesis);
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

    Manager::FunctionPtr eclClusterTrackMatchedWithCondition(const std::vector<std::string>& arguments)
    {
      if (arguments.size() <= 1) {

        std::string cutString;
        if (arguments.size() == 1)
          cutString = arguments[0];
        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));
        auto func = [cut](const Particle * particle) -> double {

          if (particle == nullptr)
            return std::numeric_limits<double>::quiet_NaN();

          const ECLCluster* cluster = particle->getECLCluster();

          if (cluster)
          {
            auto tracks = cluster->getRelationsFrom<Track>();

            for (const auto& track : tracks) {
              Particle trackParticle(&track, Belle2::Const::pion);

              if (cut->check(&trackParticle))
                return 1;
            }
            return 0;
          }
          return std::numeric_limits<double>::quiet_NaN();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function eclClusterSpecialTrackMatched");
      }
    }

    Manager::FunctionPtr averageValueInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        std::string listName = arguments[0];
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

        auto func = [listName, var](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid list name " << listName << " given to averageValueInList");
          int nParticles = listOfParticles->getListSize();
          double average = 0;
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            average += var->function(part) / nParticles;
          }
          return average;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function averageValueInList");
      }
    }

    Manager::FunctionPtr medianValueInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        std::string listName = arguments[0];
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

        auto func = [listName, var](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid list name " << listName << " given to medianValueInList");
          int nParticles = listOfParticles->getListSize();
          if (nParticles == 0)
          {
            return std::numeric_limits<double>::quiet_NaN();
          }
          std::vector<double> valuesInList;
          for (int i = 0; i < nParticles; i++)
          {
            const Particle* part = listOfParticles->getParticle(i);
            valuesInList.push_back(var->function(part));
          }
          std::sort(valuesInList.begin(), valuesInList.end());
          if (nParticles % 2 != 0)
          {
            return valuesInList[nParticles / 2];
          } else {
            return 0.5 * (valuesInList[nParticles / 2] + valuesInList[nParticles / 2 - 1]);
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function medianValueInList");
      }
    }

    Manager::FunctionPtr angleToClosestInList(const std::vector<std::string>& arguments)
    {
      // expecting the list name
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function angleToClosestInList");

      std::string listname = arguments[0];

      auto func = [listname](const Particle * particle) -> double {
        // get the list and check it's valid
        StoreObjPtr<ParticleList> list(listname);
        if (not list.isValid())
          B2FATAL("Invalid particle list name " << listname << " given to angleToClosestInList");

        // check the list isn't empty
        if (list->getListSize() == 0)
          return std::numeric_limits<double>::quiet_NaN();

        // respect the current frame and get the momentum of our input
        const auto& frame = ReferenceFrame::GetCurrent();
        const auto p_this = frame.getMomentum(particle).Vect();

        // find the particle index with the smallest opening angle
        double minAngle = 2 * M_PI;
        for (unsigned int i = 0; i < list->getListSize(); ++i)
        {
          const Particle* compareme = list->getParticle(i);
          const auto p_compare = frame.getMomentum(compareme).Vect();
          double angle = p_compare.Angle(p_this);
          if (minAngle > angle) minAngle = angle;
        }
        return minAngle;
      };
      return func;
    }

    Manager::FunctionPtr closestInList(const std::vector<std::string>& arguments)
    {
      // expecting the list name and a variable name
      if (arguments.size() != 2)
        B2FATAL("Wrong number of arguments for meta function closestInList");

      std::string listname = arguments[0];

      // the requested variable and check it exists
      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

      auto func = [listname, var](const Particle * particle) -> double {
        // get the list and check it's valid
        StoreObjPtr<ParticleList> list(listname);
        if (not list.isValid())
          B2FATAL("Invalid particle list name " << listname << " given to closestInList");

        // respect the current frame and get the momentum of our input
        const auto& frame = ReferenceFrame::GetCurrent();
        const auto p_this = frame.getMomentum(particle).Vect();

        // find the particle index with the smallest opening angle
        double minAngle = 2 * M_PI;
        int iClosest = -1;
        for (unsigned int i = 0; i < list->getListSize(); ++i)
        {
          const Particle* compareme = list->getParticle(i);
          const auto p_compare = frame.getMomentum(compareme).Vect();
          double angle = p_compare.Angle(p_this);
          if (minAngle > angle) {
            minAngle = angle;
            iClosest = i;
          }
        }

        // final check that the list wasn't empty (or some other problem)
        if (iClosest == -1) return std::numeric_limits<double>::quiet_NaN();

        return var->function(list->getParticle(iClosest));
      };
      return func;
    }

    Manager::FunctionPtr angleToMostB2BInList(const std::vector<std::string>& arguments)
    {
      // expecting the list name
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function angleToMostB2BInList");

      std::string listname = arguments[0];

      auto func = [listname](const Particle * particle) -> double {
        // get the list and check it's valid
        StoreObjPtr<ParticleList> list(listname);
        if (not list.isValid())
          B2FATAL("Invalid particle list name " << listname << " given to angleToMostB2BInList");

        // check the list isn't empty
        if (list->getListSize() == 0)
          return std::numeric_limits<double>::quiet_NaN();

        // respect the current frame and get the momentum of our input
        const auto& frame = ReferenceFrame::GetCurrent();
        const auto p_this = frame.getMomentum(particle).Vect();

        // find the most back-to-back (the largest opening angle before they
        // start getting smaller again!)
        double maxAngle = 0;
        for (unsigned int i = 0; i < list->getListSize(); ++i)
        {
          const Particle* compareme = list->getParticle(i);
          const auto p_compare = frame.getMomentum(compareme).Vect();
          double angle = p_compare.Angle(p_this);
          if (maxAngle < angle) maxAngle = angle;
        }
        return maxAngle;
      };
      return func;
    }

    Manager::FunctionPtr mostB2BInList(const std::vector<std::string>& arguments)
    {
      // expecting the list name and a variable name
      if (arguments.size() != 2)
        B2FATAL("Wrong number of arguments for meta function mostB2BInList");

      std::string listname = arguments[0];

      // the requested variable and check it exists
      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);

      auto func = [listname, var](const Particle * particle) -> double {
        // get the list and check it's valid
        StoreObjPtr<ParticleList> list(listname);
        if (not list.isValid())
          B2FATAL("Invalid particle list name " << listname << " given to mostB2BInList");

        // respect the current frame and get the momentum of our input
        const auto& frame = ReferenceFrame::GetCurrent();
        const auto p_this = frame.getMomentum(particle).Vect();

        // find the most back-to-back (the largest opening angle before they
        // start getting smaller again!)
        double maxAngle = -1.0;
        int iMostB2B = -1;
        for (unsigned int i = 0; i < list->getListSize(); ++i)
        {
          const Particle* compareme = list->getParticle(i);
          const auto p_compare = frame.getMomentum(compareme).Vect();
          double angle = p_compare.Angle(p_this);
          if (maxAngle < angle) {
            maxAngle = angle;
            iMostB2B = i;
          }
        }

        // final check that the list wasn't empty (or some other problem)
        if (iMostB2B == -1) return std::numeric_limits<double>::quiet_NaN();

        return var->function(list->getParticle(iMostB2B));
      };
      return func;
    }

    Manager::FunctionPtr maxOpeningAngleInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        std::string listName = arguments[0];
        auto func = [listName](const Particle*) -> double {
          StoreObjPtr<ParticleList> listOfParticles(listName);

          if (!(listOfParticles.isValid())) B2FATAL("Invalid Listname " << listName << " given to maxOpeningAngleInList");
          int nParticles = listOfParticles->getListSize();
          // return NaN if number of particles is less than 2
          if (nParticles < 2) return std::numeric_limits<double>::quiet_NaN();

          const auto& frame = ReferenceFrame::GetCurrent();
          double maxOpeningAngle = -1;
          for (int i = 0; i < nParticles; i++)
          {
            TVector3 v1 = frame.getMomentum(listOfParticles->getParticle(i)).Vect();
            for (int j = i + 1; j < nParticles; j++) {
              TVector3 v2 = frame.getMomentum(listOfParticles->getParticle(j)).Vect();
              const double angle = v1.Angle(v2);
              if (angle > maxOpeningAngle) maxOpeningAngle = angle;
            }
          }
          return maxOpeningAngle;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function maxOpeningAngleInList");
      }
    }

    Manager::FunctionPtr daughterCombination(const std::vector<std::string>& arguments)
    {
      // Expect 2 or more arguments.
      if (arguments.size() >= 2) {
        // First argument is the variable name
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        // Core function: calculates a variable combining an arbitrary number of particles
        auto func = [var, arguments](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            B2WARNING("Trying to access a daughter that does not exist. Skipping");
            return std::numeric_limits<float>::quiet_NaN();
          }
          const auto& frame = ReferenceFrame::GetCurrent();

          // Sum of the 4-momenta of all the selected daughters
          TLorentzVector pSum(0, 0, 0, 0);

          // Loop over the arguments. Each one of them is a generalizedIndex,
          // pointing to a particle in the decay tree.
          for (unsigned int iCoord = 1; iCoord < arguments.size(); iCoord++)
          {
            auto generalizedIndex = arguments[iCoord];
            const Particle* dauPart = particle->getParticleFromGeneralizedIndexString(generalizedIndex);
            if (dauPart)
              pSum +=  frame.getMomentum(dauPart);
            else {
              B2WARNING("Trying to access a daughter that does not exist. Index = " << generalizedIndex);
              return std::numeric_limits<float>::quiet_NaN();
            }
          }

          // Make a dummy particle out of the sum of the 4-momenta of the selected daughters
          Particle* sumOfDaughters = new Particle(pSum, 100); // 100 is one of the special numbers

          // Calculate the variable on the dummy particle
          return var->function(sumOfDaughters);
        };
        return func;
      } else
        B2FATAL("Wrong number of arguments for meta function daughterCombination");
    }



    Manager::FunctionPtr useAlternativeDaughterHypothesis(const std::vector<std::string>& arguments)
    {
      /*
       `arguments` contains the variable to calculate and a list of colon-separated index-particle pairs.
       Overall, it looks like {"M", "0:K+", "1:p+", "3:e-"}.
       The code is thus divided in two parts:
       1) Parsing. A loop over the elements of `arguments` that first separates the variable from the rest, and then splits all the index:particle
          pairs, filling a std::vector with the indexes and another one with the new mass values.
       2) Replacing: A loop over the particle's daughters. We take the 4-momentum of each of them, recalculating it with a new mass if needed, and then we calculate
          the variable value using the sum of all the 4-momenta, both updated and non-updated ones.
      */

      // Expect 2 or more arguments.
      if (arguments.size() >= 2) {

        //----
        // 1) parsing
        //----

        // First argument is the variable name
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        // Parses the other arguments, which are in the form of index:particleName pairs,
        // and stores indexes and masses in two std::vectors
        std::vector<unsigned int>indexesToBeReplaced = {};
        std::vector<double>massesToBeReplaced = {};

        // Loop over the arguments to parse them
        for (unsigned int iCoord = 1; iCoord < arguments.size(); iCoord++) {
          auto replacedDauString = arguments[iCoord];
          // Split the string in index and new mass
          std::vector<std::string> indexAndMass;
          boost::split(indexAndMass, replacedDauString, boost::is_any_of(":"));

          // Checks that the index:particleName pair is properly formatted.
          if (indexAndMass.size() > 2) {
            B2WARNING("The string indicating which daughter's mass should be replaced contains more than two elements separated by a colon. Perhaps you tried to pass a generalized index, which is not supported yet for this variable. The offending string is "
                      << replacedDauString << ", while a correct syntax looks like 0:K+.");
            return nullptr;
          }

          if (indexAndMass.size() < 2) {
            B2WARNING("The string indicating which daughter's mass should be replaced contains only one colon-separated element instead of two. The offending string is "
                      << replacedDauString << ", while a correct syntax looks like 0:K+.");
            return nullptr;
          }

          // indexAndMass[0] is the daughter index as string. Try to convert it
          int dauIndex = 0;
          try {
            dauIndex = Belle2::convertString<int>(indexAndMass[0]);
          } catch (std::invalid_argument&) {
            B2FATAL("Found the string " << indexAndMass[0] << "instead of a daughter index.");
          }

          // Determine PDG code  corresponding to indexAndMass[1] using the particle names defined in evt.pdl
          TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(indexAndMass[1].c_str());
          if (!particlePDG) {
            B2WARNING("Particle not in evt.pdl file! " << indexAndMass[1]);
            return nullptr;
          }

          // Stores the indexes and the masses in the vectors that will be passed to the lambda function
          int pdgCode = particlePDG->PdgCode();
          double dauNewMass = TDatabasePDG::Instance()->GetParticle(pdgCode)->Mass() ;
          indexesToBeReplaced.push_back(dauIndex);
          massesToBeReplaced.push_back(dauNewMass);
        } // End of parsing

        //----
        // 2) replacing
        //----

        // Core function: creates a new particle from the original one changing
        // some of the daughters' masses
        auto func = [var, indexesToBeReplaced, massesToBeReplaced](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            B2WARNING("Trying to access a particle that does not exist. Skipping");
            return std::numeric_limits<float>::quiet_NaN();
          }

          const auto& frame = ReferenceFrame::GetCurrent();

          // Sum of the 4-momenta of all the daughters with the new mass assumptions
          TLorentzVector pSum(0, 0, 0, 0);

          for (unsigned int iDau = 0; iDau < particle->getNDaughters(); iDau++)
          {
            const Particle* dauPart = particle->getDaughter(iDau);
            if (not dauPart) {
              B2WARNING("Trying to access a daughter that does not exist. Index = " << iDau);
              return std::numeric_limits<float>::quiet_NaN();
            }

            TLorentzVector dauMom =  frame.getMomentum(dauPart);

            // This can be improved with a faster algorithm to check if an std::vector contains a
            // certain element
            for (unsigned int iReplace = 0; iReplace < indexesToBeReplaced.size(); iReplace++) {
              if (indexesToBeReplaced[iReplace] == iDau) {
                double p_x = dauMom.Vect().Px();
                double p_y = dauMom.Vect().Py();
                double p_z = dauMom.Vect().Pz();
                dauMom.SetXYZM(p_x, p_y, p_z, massesToBeReplaced[iReplace]);
                break;
              }
            }
            pSum = pSum + dauMom;
          } // End of loop over number of daughter

          // Make a dummy particle out of the sum of the 4-momenta of the selected daughters
          Particle* sumOfDaughters = new Particle(pSum, 100); // 100 is one of the special numbers

          // Calculate the variable on the dummy particle
          return var->function(sumOfDaughters);
        }; // end of lambda function
        return func;
      }// end of check on number of arguments
      else
        B2FATAL("Wrong number of arguments for meta function useAlternativeDaughterHypothesis");
    }


    Manager::FunctionPtr varForFirstMCAncestorOfType(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int pdg_code = -1;
        std::string arg = arguments[0];
        std::string variable_of_interest = arguments[1];
        TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(arg.c_str());

        if (part != nullptr) {
          pdg_code = std::abs(part->PdgCode());
        } else {
          try {
            pdg_code = Belle2::convertString<int>(arg);
          } catch (std::exception& e) {}
        }

        if (pdg_code == -1) {
          B2FATAL("Ancestor " + arg + " is not recognised. Please provide valid PDG code or particle name.");
        }

        auto func = [pdg_code, variable_of_interest](const Particle * particle) -> double {
          const Particle* p = particle;

          int ancestor_level = Manager::Instance().getVariable("hasAncestor(" + std::to_string(pdg_code) + ", 0)")->function(p);
          if ((ancestor_level <= 0) or (std::isnan(ancestor_level)))
          {
            return std::numeric_limits<float>::quiet_NaN();
          }

          const MCParticle* i_p = p->getMCParticle();

          for (int a = 0; a < ancestor_level ; a = a + 1)
          {
            i_p = i_p->getMother();
          }
          const Particle* m_p = new Particle(i_p);
          return Manager::Instance().getVariable(variable_of_interest)->function(m_p);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function varForFirstMCAncestorOfType (expected 2: type and variable of interest)");
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
point numbers. Currently the only supported operations are addition (``+``),
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
                      "E.g. ``useRestFrame(daughter(0, p))`` returns the total momentum of the first daughter in its mother's rest-frame");
    REGISTER_VARIABLE("useCMSFrame(variable)", useCMSFrame,
                      "Returns the value of the variable using the CMS frame as current reference frame.\n"
                      "E.g. ``useCMSFrame(E)`` returns the energy of a particle in the CMS frame.");
    REGISTER_VARIABLE("useLabFrame(variable)", useLabFrame, R"DOC(
Returns the value of ``variable`` in the *lab* frame.

.. tip::
    The lab frame is the default reference frame, usually you don't need to use this meta-variable. 
    E.g. ``useLabFrame(E)`` returns the energy of a particle in the Lab frame, same as just ``E``.

Specifying the lab frame is useful in some corner-cases. For example:
``useRestFrame(daughter(0, formula(E - useLabFrame(E))))`` which is the difference of the first daughter's energy in the rest frame of the mother (current particle) with the same daughter's lab-frame energy.
)DOC");
    REGISTER_VARIABLE("useTagSideRecoilRestFrame(variable, daughterIndexTagB)", useTagSideRecoilRestFrame,
                      "Returns the value of the variable in the rest frame of the recoiling particle to the tag side B meson.\n"
                      "The variable should only be applied to an Upsilon(4S) list.\n"
                      "E.g. ``useTagSideRecoilRestFrame(daughter(1, daughter(1, p)), 0)`` applied on a Upsilon(4S) list (``Upsilon(4S)->B+:tag B-:sig``) returns the momentum of the second daughter of the signal B meson in the signal B meson rest frame.");
    REGISTER_VARIABLE("passesCut(cut)", passesCut,
                      "Returns 1 if particle passes the cut otherwise 0.\n"
                      "Useful if you want to write out if a particle would have passed a cut or not.\n"
                      "Returns NaN if particle is a nullptr.");
    REGISTER_VARIABLE("passesEventCut(cut)", passesEventCut,
                      "[Eventbased] Returns 1 if event passes the cut otherwise 0.\n"
                      "Useful if you want to select events passing a cut without looping into particles, such as for skimming.\n");
    REGISTER_VARIABLE("countDaughters(cut)", countDaughters,
                      "Returns number of direct daughters which satisfy the cut.\n"
                      "Used by the skimming package (for what exactly?)\n"
                      "Returns NaN if particle is a nullptr.");
    REGISTER_VARIABLE("varFor(pdgCode, variable)", varFor,
                      "Returns the value of the variable for the given particle if its abs(pdgCode) agrees with the given one.\n"
                      "E.g. ``varFor(11, p)`` returns the momentum if the particle is an electron or a positron.");
    REGISTER_VARIABLE("varForMCGen(variable)", varForMCGen,
                      "Returns the value of the variable for the given particle if the MC particle related to it is primary, not virtual, and not initial.\n"
                      "If no MC particle is related to the given particle, or the MC particle is not primary, virtual, or initial, NaN will be returned.\n"
                      "E.g. ``varForMCGen(PDG)`` returns the PDG code of the MC particle related to the given particle if it is primary, not virtual, and not initial.");
    REGISTER_VARIABLE("nParticlesInList(particleListName)", nParticlesInList,
                      "[Eventbased] Returns number of particles in the given particle List.");
    REGISTER_VARIABLE("isInList(particleListName)", isInList,
                      "Returns 1.0 if the particle is in the list provided, 0.0 if not. Note that this only checks the particle given. For daughters of composite particles, please see :b2:var:`isDaughterOfList`.");
    REGISTER_VARIABLE("isDaughterOfList(particleListNames)", isDaughterOfList,
                      "Returns 1 if the given particle is a daughter of at least one of the particles in the given particle Lists.");
    REGISTER_VARIABLE("isDescendantOfList(particleListName[, anotherParticleListName][, generationFlag = -1])", isDescendantOfList, R"DOC(
                      Returns 1 if the given particle appears in the decay chain of the particles in the given ParticleLists.

                      Passing an integer as the last argument, allows to check if the particle belongs to the specific generation:

                      * ``isDescendantOfList(<particle_list>,1)`` returns 1 if particle is a daughter of the list,
                      * ``isDescendantOfList(<particle_list>,2)`` returns 1 if particle is a granddaughter of the list,
                      * ``isDescendantOfList(<particle_list>,3)`` returns 1 if particle is a great-granddaughter of the list, etc.
                      * Default value is ``-1`` that is inclusive for all generations.
                      )DOC");
    REGISTER_VARIABLE("isMCDescendantOfList(particleListName[, anotherParticleListName][, generationFlag = -1])", isMCDescendantOfList, R"DOC(
                      Returns 1 if the given particle is linked to the same MC particle as any reconstructed daughter of the decay lists.

                      Passing an integer as the last argument, allows to check if the particle belongs to the specific generation:

                      * ``isMCDescendantOfList(<particle_list>,1)`` returns 1 if particle is matched to the same particle as any daughter of the list,
                      * ``isMCDescendantOfList(<particle_list>,2)`` returns 1 if particle is matched to the same particle as any granddaughter of the list,
                      * ``isMCDescendantOfList(<particle_list>,3)`` returns 1 if particle is matched to the same particle as any great-granddaughter of the list, etc.
                      * Default value is ``-1`` that is inclusive for all generations.

                      It makes only sense for lists created with `fillParticleListFromMC` function with ``addDaughters=True`` argument.
                      )DOC");

    REGISTER_VARIABLE("sourceObjectIsInList(particleListName)", sourceObjectIsInList, R"DOC(
Returns 1.0 if the underlying mdst object (e.g. track, or cluster) was used to create a particle in ``particleListName``, 0.0 if not. 

.. note::
  This only makes sense for particles that are not composite. Returns -1 for composite particles.
)DOC");

    REGISTER_VARIABLE("mcParticleIsInMCList(particleListName)", mcParticleIsInMCList, R"DOC(
Returns 1.0 if the particle's matched MC particle is also matched to a particle in ``particleListName`` 
(or if either of the lists were filled from generator level `modularAnalysis.fillParticleListFromMC`.)

.. seealso:: :b2:var:`isMCDescendantOfList` to check daughters.
)DOC");

    REGISTER_VARIABLE("isGrandDaughterOfList(particleListNames)", isGrandDaughterOfList,
                      "Returns 1 if the given particle is a grand daughter of at least one of the particles in the given particle Lists.");
    REGISTER_VARIABLE("daughter(i, variable)", daughter, R"DOC(
                      Returns value of variable for the i-th daughter. E.g.

                      * ``daughter(0, p)`` returns the total momentum of the first daughter.
                      * ``daughter(0, daughter(1, p)`` returns the total momentum of the second daughter of the first daughter.

                      Returns NaN if particle is nullptr or if the given daughter-index is out of bound (>= amount of daughters).
                      )DOC");
    REGISTER_VARIABLE("mcDaughter(i, variable)", mcDaughter, R"DOC(
                      Returns the value of the requested variable for the i-th Monte Carlo daughter of the particle.

                      Returns NaN if the particle is nullptr, if the particle is not matched to an MC particle,
                      or if the i-th MC daughter does not exist.

                      E.g. ``mcDaughter(0, PDG)`` will return the PDG code of the first MC daughter of the matched MC
                      particle of the reconstructed particle the function is applied to.

                      The meta variable can also be nested: ``mcDaughter(0, mcDaughter(1, PDG))``.
                      )DOC");
    REGISTER_VARIABLE("mcMother(variable)", mcMother, R"DOC(
                      Returns the value of the requested variable for the Monte Carlo mother of the particle.

                      Returns NaN if the particle is nullptr, if the particle is not matched to an MC particle,
                      or if the MC mother does not exist.

                      E.g. ``mcMother(PDG)`` will return the PDG code of the MC mother of the matched MC
                      particle of the reconstructed particle the function is applied to.

                      The meta variable can also be nested: ``mcMother(mcMother(PDG))``.
                      )DOC");
    REGISTER_VARIABLE("genParticle(index, variable)", genParticle,  R"DOC(
[Eventbased] Returns the ``variable`` for the ith generator particle.
The arguments of the function must be the ``index`` of the particle in the MCParticle Array, 
and ``variable``, the name of the function or variable for that generator particle.
If ``index`` goes beyond the length of the MCParticles array, NaN will be returned.

E.g. ``genParticle(0, p)`` returns the total momentum of the first MCParticle, which is 
the Upsilon(4S) in a generic decay.
``genParticle(0, mcDaughter(1, p)`` returns the total momentum of the second daughter of
the first MC Particle, which is the momentum of the second B meson in a generic decay.
)DOC");
    REGISTER_VARIABLE("genUpsilon4S(variable)", genUpsilon4S, R"DOC(
[Eventbased] Returns the ``variable`` evaluated for the generator-level :math:`\Upsilon(4S)`.
If no generator level :math:`\Upsilon(4S)` exists for the event, NaN will be returned.

E.g. ``genUpsilon4S(p)`` returns the total momentum of the :math:`\Upsilon(4S)` in a generic decay.
``genUpsilon4S(mcDaughter(1, p)`` returns the total momentum of the second daughter of the
generator-level :math:`\Upsilon(4S)` (i.e. the momentum of the second B meson in a generic decay.
)DOC");
    REGISTER_VARIABLE("daughterProductOf(variable)", daughterProductOf,
                      "Returns product of a variable over all daughters.\n"
                      "E.g. ``daughterProductOf(extraInfo(SignalProbability))`` returns the product of the SignalProbabilitys of all daughters.");
    REGISTER_VARIABLE("daughterSumOf(variable)", daughterSumOf,
                      "Returns sum of a variable over all daughters.\n"
                      "E.g. ``daughterSumOf(nDaughters)`` returns the number of grand-daughters.");
    REGISTER_VARIABLE("daughterLowest(variable)", daughterLowest,
                      "Returns the lowest value of the given variable among all daughters.\n"
                      "E.g. ``useCMSFrame(daughterLowest(p))`` returns the lowest momentum in CMS frame.");
    REGISTER_VARIABLE("daughterHighest(variable)", daughterHighest,
                      "Returns the highest value of the given variable among all daughters.\n"
                      "E.g. ``useCMSFrame(daughterHighest(p))`` returns the highest momentum in CMS frame.");
    REGISTER_VARIABLE("daughterDiffOf(i, j, variable)", daughterDiffOf,
                      "Returns the difference of a variable between the two given daughters.\n"
                      "E.g. ``useRestFrame(daughterDiffOf(0, 1, p))`` returns the momentum difference between first and second daughter in the rest frame of the given particle.\n"
                      "(That means that it returns :math:`p_j - p_i`)\n"
                      "Nota Bene: for the particular case 'variable=phi' you should use the :b2:var:`daughterDiffOfPhi` function.");
    REGISTER_VARIABLE("mcDaughterDiffOf(i, j, variable)", mcDaughterDiffOf,
                      "MC matched version of the `daughterDiffOf` function."); 
    REGISTER_VARIABLE("grandDaughterDiffOf(i, j, variable)", grandDaughterDiffOf,
                      "Returns the difference of a variable between the first daughters of the two given daughters.\n"
                      "E.g. ``useRestFrame(grandDaughterDiffOf(0, 1, p))`` returns the momentum difference between the first daughters of the first and second daughter in the rest frame of the given particle.\n"
                      "(That means that it returns :math:`p_j - p_i`)\n"
                      "Nota Bene: for the particular case 'variable=phi' you should use the :b2:var:`grandDaughterDiffOfPhi` function.");
    REGISTER_VARIABLE("daughterDiffOfPhi(i, j)", daughterDiffOfPhi,
                      "Returns the difference in :math:`\\phi` between the two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i`.");
    MAKE_DEPRECATED("daughterDiffOfPhi(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of two daughters can be calculated with the generic variable :b2:var:`daughterDiffOf`.)DOC");
    REGISTER_VARIABLE("mcDaughterDiffOfPhi(i, j)", mcDaughterDiffOfPhi,
                      "MC matched version of the `daughterDiffOfPhi` function."); 
    MAKE_DEPRECATED("mcDaughterDiffOfPhi(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of the MC partners of two daughters can be calculated with the generic variable :b2:var:`mcDaughterDiffOf`.)DOC");
    REGISTER_VARIABLE("grandDaughterDiffOfPhi(i, j)", grandDaughterDiffOfPhi,
                      "Returns the difference in :math:`\\phi` between the first daughters of the two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i`.\n");
    MAKE_DEPRECATED("grandDaughterDiffOfPhi(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of two granddaughters can be calculated with the generic variable :b2:var:`grandDaughterDiffOf`.)DOC");
    REGISTER_VARIABLE("daughterDiffOfClusterPhi(i, j)", daughterDiffOfClusterPhi,
                      "Returns the difference in :math:`\\phi` between the ECLClusters of two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i`.\n"
                      "The function returns NaN if at least one of the daughters is not matched to or not based on an ECLCluster.");
    MAKE_DEPRECATED("daughterDiffOfClusterPhi(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of the related ECL clusters of two daughters can be calculated with the generic variable :b2:var:`daughterDiffOf`.)DOC");
    REGISTER_VARIABLE("grandDaughterDiffOfClusterPhi(i, j)", grandDaughterDiffOfClusterPhi,
                      "Returns the difference in :math:`\\phi` between the ECLClusters of the daughters of the two given daughters.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i`.\n"
                      "The function returns NaN if at least one of the daughters is not matched to or not based on an ECLCluster.\n");
    MAKE_DEPRECATED("grandDaughterDiffOfClusterPhi(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of the related ECL clusters of two granddaughters can be calculated with the generic variable :b2:var:`grandDaughterDiffOf`.)DOC");
    REGISTER_VARIABLE("daughterDiffOfPhiCMS(i, j)", daughterDiffOfPhiCMS,
                      "Returns the difference in :math:`\\phi` between the two given daughters in the CMS frame.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i`.");
    MAKE_DEPRECATED("daughterDiffOfPhiCMS(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of two daughters in the CMS frame can be calculated with the generic variable :b2:var:`daughterDiffOf`.)DOC");
    REGISTER_VARIABLE("mcDaughterDiffOfPhiCMS(i, j)", daughterDiffOfPhiCMS,
                      "MC matched version of the `daughterDiffOfPhiCMS` function.");      
    MAKE_DEPRECATED("mcDaughterDiffOfPhiCMS(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of the MC partners of two daughters in the CMS frame can be calculated with the generic variable :b2:var:`mcDaughterDiffOf`.)DOC");
    REGISTER_VARIABLE("daughterDiffOfClusterPhiCMS(i, j)", daughterDiffOfClusterPhiCMS,
                      "Returns the difference in :math:`\\phi` between the ECLClusters of two given daughters in the CMS frame.\n"
                      "The difference is signed and takes account of the ordering of the given daughters.\n"
                      "The function returns :math:`\\phi_j - \\phi_i``.\n"
                      "The function returns NaN if at least one of the daughters is not matched to or not based on an ECLCluster.");
    MAKE_DEPRECATED("daughterDiffOfClusterPhiCMS(i, j)", false, "release-06-00-00", R"DOC(
                     The difference of the azimuthal angle :math:`\\phi` of the related ECL clusters of two daughters in the CMS frame can be calculated with the generic variable :b2:var:`daughterDiffOf`.)DOC");
    REGISTER_VARIABLE("daughterNormDiffOf(i, j, variable)", daughterNormDiffOf,
                      "Returns the normalized difference of a variable between the two given daughters.\n"
                      "E.g. ``daughterNormDiffOf(0, 1, p)`` returns the normalized momentum difference between first and second daughter in the lab frame.");
    REGISTER_VARIABLE("daughterMotherDiffOf(i, variable)", daughterMotherDiffOf,
                      "Returns the difference of a variable between the given daughter and the mother particle itself.\n"
                      "E.g. ``useRestFrame(daughterMotherDiffOf(0, p))`` returns the momentum difference between the given particle and its first daughter in the rest frame of the mother.");
    REGISTER_VARIABLE("daughterMotherNormDiffOf(i, variable)", daughterMotherNormDiffOf,
                      "Returns the normalized difference of a variable between the given daughter and the mother particle itself.\n"
                      "E.g. ``daughterMotherNormDiffOf(1, p)`` returns the normalized momentum difference between the given particle and its second daughter in the lab frame.");
    REGISTER_VARIABLE("daughterAngle(daughterIndex_1, daughterIndex_2[, daughterIndex_3])", daughterAngle, R"DOC(
                       Returns the angle in between any pair of particles belonging to the same decay tree.

                       The particles are identified via generalized daughter indexes, which are simply colon-separated lists of
                       daughter indexes, ordered starting from the root particle. For example, ``0:1:3``  identifies the fourth
                       daughter (3) of the second daughter (1) of the first daughter (0) of the mother particle. ``1`` simply
                       identifies the second daughter of the root particle.

                       Both two and three generalized indexes can be given to ``daughterAngle``. If two indices are given, the
                       variable returns the angle between the momenta of the two given particles. If three indices are given, the
                       variable returns the angle between the momentum of the third particle and a vector which is the sum of the
                       first two daughter momenta.

                       .. tip::
                           ``daughterAngle(0, 3)`` will return the angle between the first and fourth daughter.
                           ``daughterAngle(0, 1, 3)`` will return the angle between the fourth daughter and the sum of the first and
                           second daughter.
                           ``daughterAngle(0:0, 3:0)`` will return the angle between the first daughter of the first daughter, and
                           the first daughter of the fourth daughter.

                      )DOC");
    REGISTER_VARIABLE("mcDaughterAngle(daughterIndex_1, daughterIndex_2, [daughterIndex_3])", mcDaughterAngle,"MC matched version of the `daughterAngle` function.");
    REGISTER_VARIABLE("grandDaughterDecayAngle(i, j)", grandDaughterDecayAngle,
                      "Returns the decay angle of the granddaughter in the daughter particle's rest frame.\n"
                      "It is calculated with respect to the reverted momentum vector of the particle.\n"
                      "Two arguments representing the daughter and granddaughter indices have to be provided as arguments.");
    REGISTER_VARIABLE("daughterClusterAngleInBetween(i, j)", daughterClusterAngleInBetween,
                      "Returns function which returns the angle between clusters associated to the two daughters."
                      "If two indices given: returns the angle between the momenta of the clusters associated to the two given daughters."
                      "If three indices given: returns the angle between the momentum of the third particle's cluster and a vector "
                      "which is the sum of the first two daughter's cluster momenta."
                      "Returns nan if any of the daughters specified don't have an associated cluster."
                      "The arguments in the argument vector must be integers corresponding to the ith and jth (and kth) daughters.");
    REGISTER_VARIABLE("daughterInvM(i, j)", daughterInvM,
                      "Returns the invariant Mass adding the Lorentz vectors of the given daughters.\n"
                      "E.g. ``daughterInvM(0, 1, 2)`` returns the invariant Mass :math:`m = \\sqrt{(p_0 + p_1 + p_2)^2}`` of first, second and third daughter.");
    REGISTER_VARIABLE("extraInfo(name)", extraInfo,
                      "Returns extra info stored under the given name.\n"
                      "The extraInfo has to be set by a module first.\n"
                      "E.g. ``extraInfo(SignalProbability)`` returns the SignalProbability calculated by the ``MVAExpert`` module.\n"
                      "If nothing is set under the given name or if the particle is a nullptr, NaN is returned.\n"
                      "In the latter case please use `eventExtraInfo` if you want to access an EventExtraInfo variable.");
    REGISTER_VARIABLE("eventExtraInfo(name)", eventExtraInfo,
                      "[Eventbased] Returns extra info stored under the given name in the event extra info.\n"
                      "The extraInfo has to be set first by another module like MVAExpert in event mode.\n"
                      "If nothing is set under this name, NaN is returned.");
    REGISTER_VARIABLE("eventCached(variable)", eventCached,
                      "[Eventbased] Returns value of event-based variable and caches this value in the EventExtraInfo.\n"
                      "The result of second call to this variable in the same event will be provided from the cache.\n"
                      "It is recommended to use this variable in order to declare custom aliases as event-based. This is "
                      "necessary if using the eventwise mode of variablesToNtuple).");
    REGISTER_VARIABLE("particleCached(variable)", particleCached,
                      "Returns value of given variable and caches this value in the ParticleExtraInfo of the provided particle.\n"
                      "The result of second call to this variable on the same particle will be provided from the cache.");
    REGISTER_VARIABLE("modulo(variable, n)", modulo,
                      "Returns rest of division of variable by n.");
    REGISTER_VARIABLE("abs(variable)", abs,
                      "Returns absolute value of the given variable.\n"
                      "E.g. abs(mcPDG) returns the absolute value of the mcPDG, which is often useful for cuts.");
    REGISTER_VARIABLE("max(var1,var2)", max, "Returns max value of two variables.\n");
    REGISTER_VARIABLE("min(var1,var2)", min, "Returns min value of two variables.\n");
    REGISTER_VARIABLE("sin(variable)", sin, "Returns sine value of the given variable.");
    REGISTER_VARIABLE("asin(variable)", asin, "Returns arcsine of the given variable.");
    REGISTER_VARIABLE("cos(variable)", cos, "Returns cosine value of the given variable.");
    REGISTER_VARIABLE("acos(variable)", acos, "Returns arccosine value of the given variable.");
    REGISTER_VARIABLE("exp(variable)", exp, "Returns exponential evaluated for the given variable.");
    REGISTER_VARIABLE("log(variable)", log, "Returns natural logarithm evaluated for the given variable.");
    REGISTER_VARIABLE("log10(variable)", log10, "Returns base-10 logarithm evaluated for the given variable.");
    REGISTER_VARIABLE("isNAN(variable)", isNAN,
                      "Returns true if variable value evaluates to nan (determined via std::isnan(double)).\n"
                      "Useful for debugging.");
    REGISTER_VARIABLE("ifNANgiveX(variable, x)", ifNANgiveX,
                      "Returns x (has to be a number) if variable value is nan (determined via std::isnan(double)).\n"
                      "Useful for technical purposes while training MVAs.");
    REGISTER_VARIABLE("isInfinity(variable)", isInfinity,
                      "Returns true if variable value evaluates to infinity (determined via std::isinf(double)).\n"
                      "Useful for debugging.");
    REGISTER_VARIABLE("unmask(variable, flag1, flag2, ...)", unmask,
                      "unmask(variable, flag1, flag2, ...) or unmask(variable, mask) sets certain bits in the variable to zero.\n"
                      "For example, if you want to set the second, fourth and fifth bits to zero, you could call \n"
                      "``unmask(variable, 2, 8, 16)`` or ``unmask(variable, 26)``.\n"
                      "");
    REGISTER_VARIABLE("conditionalVariableSelector(cut, variableIfTrue, variableIfFalse)", conditionalVariableSelector,
                      "Returns one of the two supplied variables, depending on whether the particle passes the supplied cut.\n"
                      "The first variable is returned if the particle passes the cut, and the second variable is returned otherwise.");
    REGISTER_VARIABLE("pValueCombination(p1, p2, ...)", pValueCombination,
                      "Returns the combined p-value of the provided p-values according to the formula given in `Nucl. Instr. and Meth. A 411 (1998) 449 <https://doi.org/10.1016/S0168-9002(98)00293-9>`_ .\n"
                      "If any of the p-values is invalid, i.e. smaller than zero, -1 is returned.");
    REGISTER_VARIABLE("veto(particleList, cut, pdgCode = 11)", veto,
                      "Combines current particle with particles from the given particle list and returns 1 if the combination passes the provided cut. \n"
                      "For instance one can apply this function on a signal Photon and provide a list of all photons in the rest of event and a cut \n"
                      "around the neutral Pion mass (e.g. ``0.130 < M < 0.140``). \n"
                      "If a combination of the signal Photon with a ROE photon fits this criteria, hence looks like a neutral pion, the veto-Metavariable will return 1");
    REGISTER_VARIABLE("matchedMC(variable)", matchedMC,
                      "Returns variable output for the matched MCParticle by constructing a temporary Particle from it.\n"
                      "This may not work too well if your variable requires accessing daughters of the particle.\n"
                      "E.g. ``matchedMC(p)`` returns the total momentum of the related MCParticle.\n"
                      "Returns NaN if no matched MCParticle exists.");
    REGISTER_VARIABLE("countInList(particleList, cut='')", countInList, "[Eventbased] "
                      "Returns number of particle which pass given in cut in the specified particle list.\n"
                      "Useful for creating statistics about the number of particles in a list.\n"
                      "E.g. ``countInList(e+, isSignal == 1)`` returns the number of correctly reconstructed electrons in the event.\n"
                      "The variable is event-based and does not need a valid particle pointer as input.");
    REGISTER_VARIABLE("getVariableByRank(particleList, rankedVariableName, variableName, rank)", getVariableByRank, R"DOC(
                      Returns the value of ``variableName`` for the candidate in the ``particleList`` with the requested ``rank``.

                      .. note::
                        The `BestCandidateSelection` module available via `rankByHighest` / `rankByLowest` has to be used before.

                      .. warning::
                        The first candidate matching the given rank is used.
                        Thus, it is not recommended to use this variable in conjunction with ``allowMultiRank`` in the `BestCandidateSelection` module.

                      The suffix ``_rank`` is automatically added to the argument ``rankedVariableName``,
                      which either has to be the name of the variable used to order the candidates or the selected outputVariable name without the ending ``_rank``.
                      This means that your selected name for the rank variable has to end with ``_rank``.

                      An example of this variable's usage is given in the tutorial `B2A602-BestCandidateSelection <https://stash.desy.de/projects/B2/repos/software/browse/analysis/examples/tutorials/B2A602-BestCandidateSelection.py>`_
                      )DOC");
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
    REGISTER_VARIABLE("eclClusterSpecialTrackMatched(cut)", eclClusterTrackMatchedWithCondition,
                      "Returns if at least one Track that satisfies the given condition is related to the ECLCluster of the Particle.");
    REGISTER_VARIABLE("averageValueInList(particleListName, variable)", averageValueInList,
                      "Returns the arithmetic mean of the given variable of the particles in the given particle list.");
    REGISTER_VARIABLE("medianValueInList(particleListName, variable)", medianValueInList,
                      "Returns the median value of the given variable of the particles in the given particle list.");
    REGISTER_VARIABLE("angleToClosestInList(particleListName)", angleToClosestInList,
                      "Returns the angle between this particle and the closest particle (smallest opening angle) in the list provided.");
    REGISTER_VARIABLE("closestInList(particleListName, variable)", closestInList,
                      "Returns `variable` for the closest particle (smallest opening angle) in the list provided.");
    REGISTER_VARIABLE("angleToMostB2BInList(particleListName)", angleToMostB2BInList,
                      "Returns the angle between this particle and the most back-to-back particle (closest opening angle to 180) in the list provided.");
    REGISTER_VARIABLE("mostB2BInList(particleListName, variable)", mostB2BInList,
                      "Returns `variable` for the most back-to-back particle (closest opening angle to 180) in the list provided.");
    REGISTER_VARIABLE("maxOpeningAngleInList(particleListName)", maxOpeningAngleInList,
                      "Returns maximum opening angle in the given particle List.");
    REGISTER_VARIABLE("daughterCombination(variable, daughterIndex_1, daughterIndex_2 ... daughterIndex_n)", daughterCombination,R"DOC(
Returns a ``variable`` function only of the 4-momentum calculated on an arbitrary set of (grand)daughters. 

.. warning::
    ``variable`` can only be a function of the daughters' 4-momenta.

Daughters from different generations of the decay tree can be combined using generalized daughter indexes, which are simply colon-separated 
the list of daughter indexes, starting from the root particle: for example, ``0:1:3``  identifies the fourth 
daughter (3) of the second daughter (1) of the first daughter (0) of the mother particle.

.. tip::
    ``daughterCombination(M, 0, 3, 4)`` will return the invariant mass of the system made of the first, fourth and fifth daughter of particle. 
    ``daughterCombination(M, 0:0, 3:0)`` will return the invariant mass of the system made of the first daughter of the first daughter and the first daughter of the fourth daughter.

)DOC");
    REGISTER_VARIABLE("useAlternativeDaughterHypothesis(variable, daughterIndex_1:newMassHyp_1, ..., daughterIndex_n:newMassHyp_n)", useAlternativeDaughterHypothesis,R"DOC(
Returns a ``variable`` calculated using new mass hypotheses for (some of) the particle's daughers. 

.. warning::
    ``variable`` can only be a function of the particle 4-momentum, which is re-calculated as the sum of the daughters' 4-momenta. 
    This means that if you made a kinematic fit without updating the daughters' momenta, the result of this variable will not reflect the effect of the kinematic fit.
    Also, the track fit is not performed again: the variable only re-calculates the 4-vectors using different mass assumptions. The alternative mass assumpion is 
    used only internally by the variable, and is not stored in the datastore (i.e the daughters are not permanently changed).

.. warning::
    Generalized daughter indexes are not supported (yet!): this variable can be used only on first-generation daughters.

.. tip::
    ``useAlternativeDaughterHypothesis(M, 0:K+, 2:pi-)`` will return the invariant mass of the particle assuming that the first daughter is a kaon and the third is a pion, instead of whatever was used in reconstructing the decay. 
    ``useAlternativeDaughterHypothesis(mRecoil, 1:p+)`` will return the recoil mass of the particle assuming that the second daughter is a proton instead of whatever was used in reconstructing the decay. 

)DOC");
    REGISTER_VARIABLE("varForFirstMCAncestorOfType(type, variable)",varForFirstMCAncestorOfType,R"DOC(Returns requested variable of the first ancestor of the given type.
Ancestor type can be set up by PDG code or by particle name (check evt.pdl for valid particle names))DOC")

  }
}
