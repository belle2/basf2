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
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace Belle2 {
  namespace Variable {

    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName](const Particle * particle) -> double {
          if (particle == nullptr) {
            StoreObjPtr<EventExtraInfo> eventExtraInfo;
            return eventExtraInfo->getExtraInfo(extraInfoName);
          }
          return particle->getExtraInfo(extraInfoName);
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function extraInfo")
        return nullptr;
      }
    }

    Manager::FunctionPtr isInRegion(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        float low = 0;
        float high = 0;
        try {
          low = boost::lexical_cast<float>(arguments[1]);
          high = boost::lexical_cast<float>(arguments[2]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("Second and third argument of isInRegion meta function must be floats!");
          return nullptr;
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var, low, high](const Particle * particle) -> double {
          double result = var->function(particle);
          return (result >= low and result <= high) ? 1.0 : 0.0;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function isInRegion");
        return nullptr;
      }
    }

    Manager::FunctionPtr daughterProductOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double product = 1.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
            product *= var->function(particle->getDaughter(j));
          }
          return product;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterProductOf");
        return nullptr;
      }
    }

    Manager::FunctionPtr daughterSumOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double sum = 0.0;
          for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
            sum += var->function(particle->getDaughter(j));
          }
          return sum;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterSumOf");
        return nullptr;
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
        return nullptr;
      }
    }

    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        int daughterNumber = 0;
        try {
          daughterNumber = boost::lexical_cast<int>(arguments[0]);
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
        return nullptr;
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
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[1]);
        auto func = [var, set](const Particle * particle) -> double {
          const PIDLikelihood* pid = particle->getRelated<PIDLikelihood>();
          if (!pid)
            return -999;
          if (not pid->isAvailable(set))
            return -999;
          return var->function(particle);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function NBDeltaIfMissing");
        return nullptr;
      }
    }

//FLAVOR TAGGING RELATED:

    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightClass = arguments[1];
        auto extraInfoFromB = arguments[2];
        auto func = [particleListName, extraInfoRightClass, extraInfoFromB](const Particle*) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          Particle* target = nullptr; //Particle selected as target
          float maximum_p_track = 0; //Probability of being the target track from the track level
          float prob = 0; //The probability of beeing right classified flavor from the event level
          float maximum_q = 0; //Flavour of the track selected as target
          if (ListOfParticles->getListSize() > 0) {
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
              Particle* particle = ListOfParticles->getParticle(i);
              double x = 0;
              if (extraInfoFromB == "IsFromB(MaximumP*)") {
                x = (T.rotateLabToCms() * particle->get4Vector()).P();
              } else x = particle->getExtraInfo(extraInfoFromB);
              if (x > maximum_p_track) {
                maximum_p_track = x;
                target = particle;
              }
            }
            prob = target -> getExtraInfo(extraInfoRightClass); //Gets the probability of beeing right classified flavor from the event level
            maximum_q = target -> getCharge(); //Gets the flavor of the track selected as target
            if (extraInfoFromB == "IsFromB(Lambda)") {
              maximum_q = target->getPDGCode() / TMath::Abs(target->getPDGCode());
            }
          }
          //float r = TMath::Abs(2 * prob - 1); //Definition of the dilution factor  */
          //return 0.5 * (maximum_q * r + 1);
//            if (extraInfoFromB == ("IsFromB(IntermediateElectron)" || "IsFromB(IntermediateMuon)" || "IsFromB(SlowPion)" || "IsFromB(FSC)" || "IsFromB(Lambda)")) return -1 * maximum_q * prob;
          return maximum_q * prob;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
        return nullptr;
      }
    }

    Manager::FunctionPtr InputQrOf(const std::vector<std::string>& arguments)
    {
      //used by simple_flavor_tagger
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightClass = arguments[1];
        auto extraInfoFromB = arguments[2];
        auto func = [particleListName, extraInfoRightClass, extraInfoFromB](const Particle*) -> double {
          if (particleListName == "K+:ROE" || particleListName == "Lambda0:ROE") {
            double flavor = 0.0;
            double r = 0.0;
            double qr = 0.0;
            double final_value = 0.0;
            double val1 = 1.0;
            double val2 = 1.0;
            auto compare = [extraInfoFromB](const Particle * part1, const Particle * part2)-> bool {
              double info1 = part1->getExtraInfo(extraInfoFromB);
              double info2 = part2->getExtraInfo(extraInfoFromB);
              return (info1 > info2);
            };
            StoreObjPtr<ParticleList> ListOfParticles(particleListName);
            std::vector<const Particle*> ParticleVector;
            ParticleVector.reserve(ListOfParticles->getListSize());
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); i++) {
              ParticleVector.push_back(ListOfParticles->getParticle(i));
            }
            std::sort(ParticleVector.begin(), ParticleVector.end(), compare);
            if (particleListName == "Lambda0:ROE") {
              //Loop over Lambda vector until 3 or empty
              if (ParticleVector.size() != 0) final_value = 1.0;
              for (unsigned int i = 0; i < ParticleVector.size(); ++i) {
                //PDG Code Lambda0 3122 (indicates a B0bar)
                if (ParticleVector[i]->getPDGCode() == 3122) flavor = -1.0;
                else if (ParticleVector[i]->getPDGCode() == -3122) flavor = 1.0;
                else {flavor = 0.0;}
                r = ParticleVector[i]->getExtraInfo(extraInfoFromB);
                qr = (flavor * r);
                val1 = val1 * (1 + qr);
                val2 = val2 * (1 - qr);
              }
              final_value = (val1 - val2) / (val1 + val2);
            } else if (particleListName == "K+:ROE") {
              //Loop over K+ vector until 3 or empty
              if (ParticleVector.size() != 0) final_value = 1.0;
              for (unsigned int i = 0; i < ParticleVector.size(); i++) {
                flavor = ParticleVector[i]->getCharge();
                r = ParticleVector[i]->getExtraInfo(extraInfoFromB);
                qr = (flavor * r);
                val1 = val1 * (1 + qr);
                val2 = val2 * (1 - qr);
              }
              final_value = (val1 - val2) / (val1 + val2);
            }

            return final_value;
          }

          //SlowPion, Electron, Muon
          else{
            StoreObjPtr<EventExtraInfo> Info("EventExtraInfo");
            StoreObjPtr<ParticleList> ListOfParticles(particleListName);
            double flavor = 0.0; //Flavour of the track selected as target
            double maximum_p_track = 0.0; //Probability of being the target track from the track level
            double final_value = 0.0;
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); i++) {
              Particle* p = ListOfParticles->getParticle(i);
              double x = p->getExtraInfo(extraInfoFromB);
              if (x > maximum_p_track) {
                maximum_p_track = x;
                //In case of slowPions and intermediate momentum leptons sign is flipped
                if (particleListName == "pi+:ROE" && extraInfoFromB == "IsFromB(SlowPion)") flavor = (-1.0) * p->getCharge();
                else if (particleListName == "e+:ROE" && extraInfoFromB == "IsFromB(IntermediateElectron)") flavor = (-1.0) * p->getCharge();
                else if (particleListName == "mu+:ROE" && extraInfoFromB == "IsFromB(IntermediateMuon)") flavor = (-1.0) * p->getCharge();
                else flavor = p->getCharge();
              }
            }
            final_value = flavor * maximum_p_track;
            return final_value;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
        return nullptr;
      }
    }

    Manager::FunctionPtr IsRightClass(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * particle) -> double {
          Particle* nullpart = nullptr;
          float maximum_q = 0;
          int maximum_PDG = 0;
          int maximum_PDG_Mother = 0;
          int maximum_PDG_Mother_Mother = 0;
          const MCParticle* MCp = particle ->getRelated<MCParticle>();
          maximum_q = particle -> getCharge();
          if (MCp->getMother() != nullptr && MCp->getMother()->getMother() != nullptr) {
            maximum_PDG = TMath::Abs(MCp->getPDG());
            maximum_PDG_Mother = TMath::Abs(MCp->getMother()->getPDG());
            //for Kaons and SlowPions we need the mother of the mother for the particle
            if (particleName == "Kaon" || particleName == "SlowPion" || particleName == "Lambda" || particleName == "IntermediateElectron" || particleName == "IntermediateMuon" || particleName == "FastPion" || particleName == "FSC") maximum_PDG_Mother_Mother =  TMath::Abs(MCp->getMother()->getMother()->getPDG());
          } else {
            maximum_PDG = 0;
            maximum_PDG_Mother = 0;
          }
//           float SlowPion_q = 0;
          int SlowPion_PDG = 0;
          int SlowPion_PDG_Mother = 0;
          if (particleName == "KaonPion") {
            StoreObjPtr<ParticleList> SlowPionList("pi+:ROE");
            double maximum_prob_pi = 0;
            Particle* TargetSlowPion = nullptr;
            for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
              Particle* p_pi = SlowPionList->getParticle(i);
              double prob_pi = p_pi->getExtraInfo("IsFromB(SlowPion)");
              if (prob_pi > maximum_prob_pi) {
                maximum_prob_pi = prob_pi;
                TargetSlowPion = p_pi;
              }
              const MCParticle* MCSlowPion = TargetSlowPion ->getRelated<MCParticle>();
//               SlowPion_q = TargetSlowPion -> getCharge();
              if (MCSlowPion->getMother() != nullptr && MCSlowPion->getMother()->getMother() != nullptr) {
                SlowPion_PDG = TMath::Abs(MCSlowPion->getPDG());
                SlowPion_PDG_Mother = TMath::Abs(MCSlowPion->getMother()->getPDG());
              }
            }
          }
//           float FastParticle_q = 0;
          int FastParticle_PDG_Mother = 0;
          if (particleName == "FSC") {
            StoreObjPtr<ParticleList> FastParticleList("pi+:ROE");
            PCmsLabTransform T;
            double maximum_prob_fast = 0;
            Particle* TargetFastParticle = nullptr;
            for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
              Particle* p_fast = FastParticleList->getParticle(i);
              double prob_fast = (T.rotateLabToCms() * p_fast -> get4Vector()).P();
              if (prob_fast > maximum_prob_fast) {
                maximum_prob_fast = prob_fast;
                TargetFastParticle = p_fast;
              }
              const MCParticle* MCFastParticle = TargetFastParticle ->getRelated<MCParticle>();
//               FastParticle_q = TargetFastParticle -> getCharge();
              if (MCFastParticle->getMother() != nullptr && MCFastParticle->getMother()->getMother() != nullptr) {
                FastParticle_PDG_Mother = TMath::Abs(MCFastParticle->getMother()->getPDG());
              }
            }
          }
          if (particleName == "Electron"
              && ((maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 11 && maximum_PDG_Mother == 511)
                  || (maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 11 && maximum_PDG_Mother_Mother == 511))) {
            return 1.0;
          } else if (particleName == "IntermediateElectron"
                     && maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 11 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          } else if (particleName == "Muon"
                     && ((maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 13 && maximum_PDG_Mother == 511)
                         || (maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 13 && maximum_PDG_Mother_Mother == 511))) {
            return 1.0;
          } else if (particleName == "IntermediateMuon"
                     && maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 13 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          }  else if (particleName == "KinLepton"
                      && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && (maximum_PDG == 11 || maximum_PDG == 13) && maximum_PDG_Mother == 511) {
            return 1.0;
          } else if (particleName == "Kaon"
                     && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 321 && maximum_PDG_Mother > 400 && maximum_PDG_Mother < 500 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          } else if (particleName == "SlowPion"
                     && maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 211 && maximum_PDG_Mother == 413 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          } else if (particleName == "KaonPion"
                     && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 321 && SlowPion_PDG == 211 && maximum_PDG_Mother == SlowPion_PDG_Mother) {
            return 1.0;
          } else if (particleName == "FastPion"
                     && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 211 && maximum_PDG_Mother == 511) {
            return 1.0;
          } else if (particleName == "MaximumP*"
                     && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)) {
            return 1.0;
          } else if (particleName == "FSC"
                     && maximum_q != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 211 && FastParticle_PDG_Mother == 511) {
            return 1.0;
          } else if (particleName == "Lambda"
                     && (particle->getPDGCode() / TMath::Abs(particle->getPDGCode())) != Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
                     && maximum_PDG == 3122) {
            return 1.0;
          } else {
            return 0.0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function IsRightClass");
        return nullptr;
      }
    }

    Manager::FunctionPtr IsFromB(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * part) -> double {
          const MCParticle* mcParticle = part->getRelated<MCParticle>();
          if (mcParticle == nullptr) return 0.0;
          //direct electron
          else if (particleName == "Electron"
          && ((mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          || (mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511))) {
            return 1.0;
            //intermediate electron
          } else if (particleName == "IntermediateElectron"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
            //direct muon
          } else if (particleName == "Muon"
          && ((mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          || (mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511))
                    ) {
            return 1.0;
            //intermediate muon
          } else if (particleName == "IntermediateMuon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
            //KinLepton
          } else if (particleName == "KinLepton"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && (TMath::Abs(mcParticle->getPDG()) == 13 || TMath::Abs(mcParticle->getPDG()) == 11)
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
            //kaon
          } else if (particleName == "Kaon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 321
          && TMath::Abs(mcParticle->getMother()->getPDG()) > 400
          && TMath::Abs(mcParticle->getMother()->getPDG()) < 500
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
            //slow pion
          } else if (particleName == "SlowPion"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 211
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 413
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
            //high momentum pions
          } else if (particleName == "FastPion"
          && mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 211
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511) {
            return 1.0;
            //lambdas
          } else if (particleName == "Lambda"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 3122) {
            return 1.0;
          } else return 0.0;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function IsFromB");
        return nullptr;
      }
    }

    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          double maximum_prob = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
            Particle* p = ListOfParticles->getParticle(i);
            double prob = 0;
            if (extraInfoName == "IsFromB(MaximumP*)") {
              prob = (T.rotateLabToCms() * p->get4Vector()).P();
            } else prob = p->getExtraInfo(extraInfoName);
            if (prob > maximum_prob) {
              maximum_prob = prob;
            }
          }
          if ((extraInfoName == "IsFromB(MaximumP*)" && (T.rotateLabToCms() * particle -> get4Vector()).P() == maximum_prob) ||
              (extraInfoName != "IsFromB(MaximumP*)" && particle -> getExtraInfo(extraInfoName) == maximum_prob)) {
            return 1.0;
          } else return 0.0;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
        return nullptr;
      }
    }

    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle*) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          double maximum_prob = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
            Particle* p = ListOfParticles->getParticle(i);
            double prob = p->getExtraInfo(extraInfoName);
            if (prob > maximum_prob) {
              maximum_prob = prob;
            }
          }
          return maximum_prob;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
        return nullptr;
      }
    }

    Manager::FunctionPtr SemiLeptonicVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          PCmsLabTransform T;
          TLorentzVector momXchargedtracks; //Momentum of charged X tracks in CMS-System
          TLorentzVector momXchargedclusters; //Momentum of charged X clusters in CMS-System
          TLorentzVector momXneutralclusters; //Momentum of neutral X clusters in CMS-System
          TLorentzVector momTarget = T.rotateLabToCms() * particle -> get4Vector();  //Momentum of Mu in CMS-System
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          const auto& tracks = roe->getTracks();
          for (auto & x : tracks) {
            const TrackFitResult* tracki = x->getTrackFitResult(x->getRelated<PIDLikelihood>()->getMostLikely());
            if (tracki == nullptr || particle->getTrack() == x) continue;
            TLorentzVector momtrack(tracki->getMomentum(), 0);
            momXchargedtracks += T.rotateLabToCms() * momtrack;
          }
          const auto& ecl = roe->getECLClusters();
          for (auto & x : ecl) {
            if (x->isNeutral()) momXneutralclusters += T.rotateLabToCms() * x -> get4Vector();
            if (!(x->isNeutral())) momXchargedclusters += T.rotateLabToCms() * x -> get4Vector();
          }
          const auto& klm = roe->getKLMClusters();
          for (auto & x : klm) {
            if (!(x -> getAssociatedTrackFlag()) && !(x -> getAssociatedEclClusterFlag())) {
              momXneutralclusters += T.rotateLabToCms() * x -> getMomentum();
            }
          }
          TLorentzVector momXcharged(momXchargedtracks.Vect(), momXchargedclusters.E());
          TLorentzVector momX = (momXcharged + momXneutralclusters) - momTarget; //Total Momentum of the recoiling X in CMS-System
          TLorentzVector momMiss = -(momX + momTarget); //Momentum of Anti-v  in CMS-System
          if (requestedVariable == "recoilMass") return momX.M();
          else if (requestedVariable == "p_missing_CMS") return momMiss.Vect().Mag();
          else if (requestedVariable == "CosTheta_missing_CMS") return TMath::Cos(momTarget.Angle(momMiss.Vect()));
          else if (requestedVariable == "EW90") {
            TLorentzVector momW = momTarget + momMiss; //Momentum of the W-Boson in CMS
            float E_W_90 = 0 ; // Energy of all charged and neutral clusters in the hemisphere of the W-Boson
            for (auto & i : ecl) {
              if ((T.rotateLabToCms() * i -> get4Vector()).Vect().Dot(momW.Vect()) > 0) E_W_90 += i -> getEnergy();
            }
//       for (auto & i : klm) {
//         if ((T.rotateLabToCms() * i -> getMomentum()).Vect().Dot(momW.Vect()) > 0) E_W_90 +=;
//         }
            return E_W_90;
          } else {
            B2FATAL("Wrong variable requested. The possibilities are recoilMass, p_missing_CMS, CosTheta_missing_CMS or EW90");
            return 0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function SemiLeptonicVariables");
        return nullptr;
      }
    }

    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> FastParticleList("pi+:ROE");
          PCmsLabTransform T;
          double maximum_prob_fast = 0;
          Particle* TargetFastParticle = nullptr;
          for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
            Particle* p_fast = FastParticleList->getParticle(i);
            double prob_fast = (T.rotateLabToCms() * p_fast -> get4Vector()).P();
            if (prob_fast > maximum_prob_fast) {
              maximum_prob_fast = prob_fast;
              TargetFastParticle = p_fast;
            }
          }
          TLorentzVector momSlowPion = T.rotateLabToCms() * particle -> get4Vector();  //Momentum of Slow Pion in CMS-System
          TLorentzVector momFastParticle = T.rotateLabToCms() * TargetFastParticle -> get4Vector();  //Momentum of Slow Pion in CMS-System

          if (requestedVariable == "p_CMS_Fast") return momFastParticle.P();
          else if (requestedVariable == "cosSlowFast") return TMath::Cos(momSlowPion.Angle(momFastParticle.Vect()));
          else if (requestedVariable == "cosTPTO_Fast") return Variable::Manager::Instance().getVariable("cosTPTO")->function(TargetFastParticle);
          else if (requestedVariable == "SlowFastHaveOpositeCharges") {
            if (particle->getCharge()*TargetFastParticle->getCharge() == -1) {
              return 1;
            } else return 0;
          } else {
            B2FATAL("Wrong variable requested. The possibilities are p_CMS_Fast, cosSlowFast or cosTPTO_Fast");
            return 0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function FSCVariables");
        return nullptr;
      }
    }

    Manager::FunctionPtr CheckingVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto ListName = arguments[0];
        auto requestedVariable = arguments[1];
        auto func = [requestedVariable, ListName](const Particle*) -> double {
          if (requestedVariable == "getListSize") {
            StoreObjPtr<ParticleList> ListOfParticles(ListName);
            return ListOfParticles->getListSize();
          } else {
            B2FATAL("Wrong requested Variable. Available is getListSize for particle lists");
            return 0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function CheckingVariables");
        return nullptr;
      }
    }

    Manager::FunctionPtr IsDaughterOf(const std::vector<std::string>& arguments)
    {
      auto motherlist = arguments[0];
      auto func = [motherlist](const Particle * particle) -> double {

        StoreObjPtr<ParticleList> Y(motherlist);
        std::vector<Particle*> daughters;
        for (unsigned int i = 0; i < Y->getListSize(); ++i) {
          const auto& x = Y->getParticle(i)->getDaughters();
          daughters.insert(daughters.end(), x.begin(), x.end());
        }

        while (!daughters.empty()) {
          std::vector<Particle*> tmpdaughters;
          for (auto & d : daughters) {
            if (d == particle)
              return 1.0;
            const auto& x = d->getDaughters();
            tmpdaughters.insert(tmpdaughters.end(), x.begin(), x.end());
          }
          daughters = tmpdaughters;
        }
        return 0.0;
      };
      return func;
    }

    // Continuum Suppression related -----------------------------------------------
    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto variableName = arguments[0];
        // auto useFinalState = arguments[1];
        int index = -1;

        // Next make FS0 (aka k0) default.
        // If user specifies FS1 in arguments[1], overwrite k0 with k1 in loop.
        std::vector<std::string> names = {"k0mm2",   "k0et",
                                          "k0hso00", "k0hso01", "k0hso02", "k0hso03", "k0hso04",
                                          "k0hso10", "k0hso12", "k0hso14",
                                          "k0hso20", "k0hso22", "k0hso24",
                                          "k0hoo0",  "k0hoo1",  "k0hoo2",  "k0hoo3",  "k0hoo4"
                                         };

        for (unsigned i = 0; i < names.size(); ++i) {
          if (variableName == names[i])
            index = i;
        }

        auto func = [index](const Particle * particle) -> double {
          const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
          std::vector<float> ksfwFS0 = qq->getKsfwFS0();
          return ksfwFS0.at(index);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function KSFWVariables");
        return nullptr;
      }
    }

    VARIABLE_GROUP("MetaFunctions");
    REGISTER_VARIABLE("isInRegion(variable, low, high)", isInRegion, "Returns 1 if given variable is inside a given region. Otherwise 0.");
    REGISTER_VARIABLE("daughter(n, variable)", daughter, "Returns value of variable for the nth daughter.");
    REGISTER_VARIABLE("daughterProductOf(variable)", daughterProductOf, "Returns product of a variable over all daughters.");
    REGISTER_VARIABLE("daughterSumOf(variable)", daughterSumOf, "Returns sum of a variable over all daughters.");
    REGISTER_VARIABLE("extraInfo(name)", extraInfo, "Returns extra info stored under the given name.");
    REGISTER_VARIABLE("getExtraInfo(name)", extraInfo, "Deprecated, use extraInfo(name) instead.");
    REGISTER_VARIABLE("abs(variable)", abs, "Returns absolute value of the given variable.");
    REGISTER_VARIABLE("NBDeltaIfMissing(dectector, pid_variable)", NBDeltaIfMissing, "Returns -999 (delta function of NeuroBayes) instead of variable value if pid from given detector is missing.");
    REGISTER_VARIABLE("IsDaughterOf(variable)", IsDaughterOf, "Check if the particle is a daughter of the given list.");
    REGISTER_VARIABLE("KSFWVariables(variable)", KSFWVariables, "Returns et, mm2, or one of the 16 KSFW moments.");

    VARIABLE_GROUP("MetaFunctions FlavorTagging")
    REGISTER_VARIABLE("InputQrOf(particleListName, extraInfoRightClass, extraInfoFromB)", InputQrOf, "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("QrOf(particleListName, extraInfoRightClass, extraInfoFromB)", QrOf, "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("IsRightClass(particleName)", IsRightClass, "FlavorTagging: returns 1 if the class track by particleName category has the same flavor as the MC target track 0 else also if there is no target track");
    REGISTER_VARIABLE("IsFromB(particleName)", IsFromB, "Checks if the given Particle was really from a B. 1.0 if true otherwise 0.0");
    REGISTER_VARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat, "Returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName. The probability is accessed via extraInfoName.");
    REGISTER_VARIABLE("SemiLeptonicVariables(requestedVariable)", SemiLeptonicVariables, "FlavorTagging:[Eventbased] Kinematical variables (recoilMass, p_missing_CMS, CosTheta_missing_CMS or EW90) assuming a semileptonic decay with the given particle as target.");
    REGISTER_VARIABLE("FSCVariables(requestedVariable)", FSCVariables, "FlavorTagging:[Eventbased] Kinematical variables for FastSlowCorrelated category (p_CMS_Fast, cosSlowFast, SlowFastHaveOpositeCharges, or cosTPTO_Fast).");
    REGISTER_VARIABLE("CheckingVariables(ListName, requestedVariable)", CheckingVariables, "FlavorTagging:[Eventbased] Available checking variables are getListSize for particle lists.");
    REGISTER_VARIABLE("HighestProbInCat(particleListName, extraInfoName)", HighestProbInCat, "Returns the highest probability value for the given category")

  }
}
