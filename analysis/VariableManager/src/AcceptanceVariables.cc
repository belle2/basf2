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
#include <analysis/VariableManager/AcceptanceVariables.h>

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
      if (theta < 25.)  return 0;
      if (theta < 40.)  return 1; //forward
      if (theta < 129.) return 2; //barrel
      if (theta < 155.) return 3; //backwards
      else return 0;
    }

    double thetaInBKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2) {
        return 1;
      } else return 0;
    }

    double thetaInEKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 1 || acceptance == 3) {
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

    REGISTER_VARIABLE("thetaInCDCAcceptance",   thetaInCDCAcceptance, "Particle is within CDC angular acceptance.");
    REGISTER_VARIABLE("thetaInTOPAcceptance",   thetaInTOPAcceptance, "Particle is within TOP angular acceptance.");
    REGISTER_VARIABLE("thetaInARICHAcceptance", thetaInARICHAcceptance, "Particle is within ARICH angular acceptance.");
    REGISTER_VARIABLE("thetaInECLAcceptance",   thetaInECLAcceptance,
                      "Particle is within ECL angular acceptance. 1: Forward; 2: Barrel; 3: Backwards.");
    REGISTER_VARIABLE("thetaInBECLAcceptance",  thetaInBECLAcceptance, "Particle is within Barrel ECL angular acceptance.");
    REGISTER_VARIABLE("thetaInEECLAcceptance",  thetaInEECLAcceptance, "Particle is within Endcap ECL angular acceptance.");
    REGISTER_VARIABLE("thetaInKLMAcceptance",   thetaInKLMAcceptance,
                      "Particle is within KLM angular acceptance. 1: Forward; 2: Barrel; 3: Backwards.");
    REGISTER_VARIABLE("thetaInBKLMAcceptance",  thetaInBKLMAcceptance, "Particle is within Barrel KLM angular acceptance.");
    REGISTER_VARIABLE("thetaInEKLMAcceptance",  thetaInEKLMAcceptance, "Particle is within Endcap KLM angular acceptance.");

    REGISTER_VARIABLE("ptInTOPAcceptance",   ptInTOPAcceptance,  "Particle is within TOP transverse momentum acceptance.");
    REGISTER_VARIABLE("ptInBECLAcceptance",  ptInBECLAcceptance, "Particle is within Barrel ECL transverse momentum acceptance.");
    REGISTER_VARIABLE("ptInBKLMAcceptance",  ptInBKLMAcceptance, "Particle is within Barrel KLM transverse momentum acceptance.");

    REGISTER_VARIABLE("inCDCAcceptance",   inCDCAcceptance,   "Particle is within CDC geometrical acceptance.");
    REGISTER_VARIABLE("inTOPAcceptance",   inTOPAcceptance,   "Particle is within TOP geometrical acceptance.");
    REGISTER_VARIABLE("inARICHAcceptance", inARICHAcceptance, "Particle is within ARICH geometrical acceptance.");
    REGISTER_VARIABLE("inECLAcceptance",   inECLAcceptance,   "Particle is within ECL geometrical acceptance.");
    REGISTER_VARIABLE("inKLMAcceptance",   inKLMAcceptance,   "Particle is within KLM geometrical acceptance.");

  }
}
