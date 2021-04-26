/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Francesco Tenchini                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/AcceptanceVariables.h>
#include <analysis/VariableManager/Manager.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //Theta Acceptance

    double thetaInCDCAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 17. && theta < 150.) {
        return 1;
      } else return 0;
    }

    double thetaInTOPAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 31. && theta < 128.) {
        return 1;
      } else return 0;
    }

    double thetaInARICHAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 14. && theta < 30.) {
        return 1;
      } else return 0;
    }

    double thetaInECLAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 12.4 && theta < 31.4) { //forward
        return 1;
      } else if (theta > 32.2 && theta < 128.7) { //barrel
        return 2;
      } else if (theta > 130.7 && theta < 155.1) { //backwards
        return 3;
      } else return 0;
    }

    double thetaInBECLAcceptance(const Particle* particle)
    {
      double acceptance = thetaInECLAcceptance(particle);
      if (acceptance == 2) {
        return 1;
      } else return 0;
    }

    double thetaInEECLAcceptance(const Particle* particle)
    {
      double acceptance = thetaInECLAcceptance(particle);
      if (acceptance == 1 || acceptance == 3) {
        return 1;
      } else return 0;
    }

    double thetaInKLMAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta < 18.)  return 0;
      if (theta < 37.)  return 1; //forward endcap
      if (theta < 47.)  return 2; //forward overlap
      if (theta < 122.) return 3; //barrel
      if (theta < 130.) return 4; //backward overlap
      if (theta < 155.) return 5; //backward endcap
      else return 0;
    }

    double thetaInBKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2 || acceptance == 3 || acceptance == 4) {
        return 1;
      } else return 0;
    }

    double thetaInEKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance != 0 && acceptance != 3) {
        return 1;
      } else return 0;
    }

    double thetaInKLMOverlapAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2 || acceptance == 4) {
        return 1;
      } else return 0;
    }

    //Pt Acceptance

    double ptInTOPAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.27)  return 1;
      else  return 0;
    }

    double ptInBECLAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.28)  return 1;
      else  return 0;
    }

    double ptInBKLMAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.6)  return 1;
      else  return 0;
    }

    //Combined Acceptance

    double inCDCAcceptance(const Particle* particle)
    {
      return thetaInCDCAcceptance(particle);
    }

    double inTOPAcceptance(const Particle* particle)
    {
      return (thetaInTOPAcceptance(particle) && ptInTOPAcceptance(particle));
    }

    double inARICHAcceptance(const Particle* particle)
    {
      return thetaInARICHAcceptance(particle);
    }

    double inECLAcceptance(const Particle* particle)
    {
      return (thetaInEECLAcceptance(particle) || (thetaInBECLAcceptance(particle) && ptInBECLAcceptance(particle)));
    }

    double inKLMAcceptance(const Particle* particle)
    {
      return (thetaInEKLMAcceptance(particle) || (thetaInBKLMAcceptance(particle) && ptInBKLMAcceptance(particle)));
    }

    // ---

    VARIABLE_GROUP("Acceptance");

    REGISTER_VARIABLE("thetaInCDCAcceptance",   thetaInCDCAcceptance,
                      "Returns 1.0 if particle is within CDC angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInTOPAcceptance",   thetaInTOPAcceptance,
                      "Returns 1.0 if particle is within TOP angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInARICHAcceptance", thetaInARICHAcceptance,
                      "Returns 1.0 if particle is within ARICH angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInECLAcceptance",   thetaInECLAcceptance, R"DOC(
Checks if particle is within ECL angular acceptance. Return values are the following:

* 1: Forward; 
* 2: Barrel; 
* 3: Backwards.
)DOC");
    REGISTER_VARIABLE("thetaInBECLAcceptance",  thetaInBECLAcceptance, "Returns 1.0 if particle is within Barrel ECL angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInEECLAcceptance",  thetaInEECLAcceptance, "Returns 1.0 if particle is within Endcap ECL angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInKLMAcceptance",   thetaInKLMAcceptance,  R"DOC(
Checks if particle is within KLM angular acceptance. Return values are the following:

* 1: Forward endcap; 
* 2: Forward overlap; 
* 3: Barrel; 
* 4: Backward overlap; 
* 5: Backward endcap.
)DOC");

    REGISTER_VARIABLE("thetaInBKLMAcceptance",  thetaInBKLMAcceptance, "Returns 1.0 if particle is within Barrel KLM angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInEKLMAcceptance",  thetaInEKLMAcceptance, "Returns 1.0 if particle is within Endcap KLM angular acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("thetaInKLMOverlapAcceptance",  thetaInKLMOverlapAcceptance,
                      "Returns 1.0 if particle is within the angular region where KLM barrel and endcaps overlap, 0.0 otherwise.");

    REGISTER_VARIABLE("ptInTOPAcceptance",   ptInTOPAcceptance,  "Returns 1.0 if particle is within TOP transverse momentum acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("ptInBECLAcceptance",  ptInBECLAcceptance, "Returns 1.0 if particle is within Barrel ECL transverse momentum acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("ptInBKLMAcceptance",  ptInBKLMAcceptance, "Returns 1.0 if particle is within Barrel KLM transverse momentum acceptance, 0.0 otherwise.");

    REGISTER_VARIABLE("inCDCAcceptance",   inCDCAcceptance,   "Returns 1.0 if particle is within CDC geometrical and kinematical acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("inTOPAcceptance",   inTOPAcceptance,   "Returns 1.0 if particle is within TOP geometrical and kinematical acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("inARICHAcceptance", inARICHAcceptance, "Returns 1.0 if particle is within ARICH geometrical and kinematical acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("inECLAcceptance",   inECLAcceptance,   "Returns 1.0 if particle is within ECL geometrical and kinematical acceptance, 0.0 otherwise.");
    REGISTER_VARIABLE("inKLMAcceptance",   inKLMAcceptance,   "Returns 1.0 if particle is within KLM geometrical and kinematical acceptance, 0.0 otherwise.");

  }
}
