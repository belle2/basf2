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

    Manager::FunctionPtr getExtraInfo(const std::vector<std::string>& arguments)
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
        B2WARNING("Wrong number of arguments for meta function getExtraInfo")
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
    Manager::FunctionPtr bestQrOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle*) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          double maximum_q = 0;
          double maximum_r = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
            Particle* p = ListOfParticles->getParticle(i);
            double r = p->getExtraInfo(extraInfoName);
            if (r > maximum_r) {
              maximum_r = r;
              maximum_q = p->getCharge();
            }
          }
          return maximum_r * maximum_q;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function bestQrOf");
        return nullptr;
      }
    }

    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightClass = arguments[1];
        auto extraInfoFromB = arguments[2];
        auto func = [particleListName, extraInfoRightClass, extraInfoFromB](const Particle*) -> double {
          StoreObjPtr<EventExtraInfo> Info("EventExtraInfo");
          float p = Info -> getExtraInfo(extraInfoRightClass); //Gets the probability of beeing right classified flavour from the event level
          float r = TMath::Abs(2 * p - 1); //Definition of the dilution factor
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          float maximum_q = 0; //Flavour of the track selected as target
          float maximum_p_track = 0; //Probability of being the target track from the track level
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
            Particle* p = ListOfParticles->getParticle(i);
            float x = p->getExtraInfo(extraInfoFromB);
            if (x > maximum_p_track) {
              maximum_p_track = x;
              maximum_q = p->getCharge();
            }
          }
          return 0.5 * (maximum_q * r + 1);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
        return nullptr;
      }
    }

    Manager::FunctionPtr IsRightClass(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        auto particleName = arguments[0];
        auto particleListName = arguments[1];
        auto extraInfoName = arguments[2];
        auto func = [particleName, particleListName, extraInfoName](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
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
            if (particleName == "Kaon" || particleName == "SlowPion") maximum_PDG_Mother_Mother =  TMath::Abs(MCp->getMother()->getMother()->getPDG());
          } else {
            maximum_PDG = 0;
            maximum_PDG_Mother = 0;
          }

          if (particleName == "Electron"
          && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 11 && maximum_PDG_Mother == 511) {
            return 1.0;
          } else if (particleName == "Muon"
          && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart) && maximum_PDG == 13 && maximum_PDG_Mother == 511) {
            return 1.0;
          } else if (particleName == "Kaon"
          && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
          && maximum_PDG == 321 && maximum_PDG_Mother > 400 && maximum_PDG_Mother < 500 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          } else if (particleName == "SlowPion"
          && maximum_q == Variable::Manager::Instance().getVariable("isRestOfEventB0Flavor")->function(nullpart)
          && maximum_PDG == 211 && maximum_PDG_Mother == 413 && maximum_PDG_Mother_Mother == 511) {
            return 1.0;
          } else {
            return 0.0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function IsRightClass");
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
          else if (particleName == "Electron"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511) {
            return 1.0;
          } else if (particleName == "Muon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511) {
            return 1.0;
          } else if (particleName == "Kaon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 321
          && TMath::Abs(mcParticle->getMother()->getPDG()) > 400
          && TMath::Abs(mcParticle->getMother()->getPDG()) < 500
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
          } else if (particleName == "SlowPion"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 211
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 413
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
            return 1.0;
          } else if (particleName == "Lambda"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 3122
          //&& TMath::Abs(mcParticle->getMother()->getPDG()) == 4122 //do we really need that kind of information?
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511) {
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
          double maximum_prob = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
            Particle* p = ListOfParticles->getParticle(i);
            double prob = p->getExtraInfo(extraInfoName);
            if (prob > maximum_prob) {
              maximum_prob = prob;
            }
          }
          if (particle -> getExtraInfo(extraInfoName) == maximum_prob) return 1.0;
          else return 0.0;
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
        B2FATAL("Wrong number of arguments (1 required) for meta function recoilMassBtag");
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

    VARIABLE_GROUP("MetaFunctions");
    REGISTER_VARIABLE("isInRegion(variable, low, high)", isInRegion, "Returns 1 if given variable is inside a given region. Otherwise 0.");
    REGISTER_VARIABLE("daughter(n, variable)", daughter, "Returns value of variable for the nth daughter.");
    REGISTER_VARIABLE("daughterProductOf(variable)", daughterProductOf, "Returns product of a variable over all daughters.");
    REGISTER_VARIABLE("daughterSumOf(variable)", daughterSumOf, "Returns sum of a variable over all daughters.");
    REGISTER_VARIABLE("getExtraInfo(name)", getExtraInfo, "Returns extra info stored under the given name.");
    REGISTER_VARIABLE("abs(variable)", abs, "Returns absolute value of the given variable.");
    REGISTER_VARIABLE("NBDeltaIfMissing(dectector, pid_variable)", NBDeltaIfMissing, "Returns -999 (delta function of NeuroBayes) instead of variable value if pid from given detector is missing.");
    REGISTER_VARIABLE("IsDaughterOf(variable)", IsDaughterOf, "Check if the particle is a daughter of the given list.");

    VARIABLE_GROUP("MetaFunctions FlavorTagging")
    REGISTER_VARIABLE("bestQrOf(particleListName, extraInfoName)", bestQrOf, "FlavorTagging:[Eventbased] q*r where r is maximum from extraInfoName in particlelistName.");
    REGISTER_VARIABLE("QrOf(particleListName, extraInfoRightClass, extraInfoFromB)", QrOf, "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("IsRightClass(particleName, particleListName, extraInfoName)", IsRightClass, "FlavorTagging: returns 1 if the class track by particleName category has the same flavour as the MC target track 0 else also if there is no target track");
    REGISTER_VARIABLE("IsFromB(particleName)", IsFromB, "Checks if the given Particle was really from a B. 1.0 if true otherwise 0.0");
    REGISTER_VARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat, "Returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName. The probability is accessed via extraInfoName.");
    REGISTER_VARIABLE("SemiLeptonicVariables(requestedVariable)", SemiLeptonicVariables, "FlavorTagging:[Eventbased] Kinematical variables (recoilMass, p_missing_CMS, CosTheta_missing_CMS or EW90) assuming a semileptonic decay with the given particle as target.");
  }
}
