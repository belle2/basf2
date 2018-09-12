/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej, Anze Zupanc                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/ParticleDaughterVariables.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/MCTruthVariables.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double hasCharmedDaughter(const Particle* particle, const std::vector<double>& transition)
    {
      double Status = 0.0;

      // Check if correct arguments
      if (abs(transition[0]) != 1) {
        B2ERROR("The parameter variable hasCharmedDaughter() only accepts 1 or -1 as an argument.");
        return -999.0;
      }

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return -999.0;
      }

      // Check if MC particle exists
      const MCParticle* mcp = particle->getRelated<MCParticle>();
      if (!mcp)
        return -999.0;

      // MCParticle should be related to a B meson
      if (abs(mcp->getPDG()) != 511 and abs(mcp->getPDG()) != 521)
        return -999.0;

      // Check if the particle has daughters
      int nDaughters = int(mcp->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return -999.0;
      }

      // Get the PDG sign and load daughters
      int motherPDGSign = (particle->getPDGCode()) / (abs(particle->getPDGCode()));
      std::vector<MCParticle*> mcDaughters = mcp->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = mcDaughters[iDaughter]->getPDG();
        int daughterPDGSign = daughterPDG / (abs(daughterPDG));

        if (transition[0] == 1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4)
              && motherPDGSign == daughterPDGSign) // charmed meson or baryon and b->anti-c transition
            Status = 1.0;
        } else if (transition[0] == -1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4)
              && motherPDGSign == -daughterPDGSign) // charmed meson or baryon and b->c transition
            Status = 1.0;
        }
      }

      return Status;
    }

    double hasCharmoniumDaughter(const Particle* particle)
    {
      double Status = 0.0;

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return -999.0;
      }

      // Check if MC particle exists
      const MCParticle* mcp = particle->getRelated<MCParticle>();
      if (!mcp)
        return -999.0;

      // MCParticle should be related to a B meson
      if (abs(mcp->getPDG()) != 511 and abs(mcp->getPDG()) != 521)
        return -999.0;

      // Check if the particle has daughters
      int nDaughters = int(mcp->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return -999.0;
      }

      // Load daughters
      std::vector<MCParticle*> mcDaughters = mcp->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = mcDaughters[iDaughter]->getPDG();
        if ((abs(daughterPDG) / 10) % 10 == 4 && (abs(daughterPDG) / 100) % 10 == 4) // charmonium state: b->c anti-c q transition
          Status = 1.0;
      }

      return Status;
    }

    double hasRealPhotonDaughter(const Particle* particle)
    {
      double Status = 0.0;

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return -999.0;
      }

      // Check if the particle has daughters
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return -999.0;
      }

      // Load daughters
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        double photosFlag = particleMCPhotosParticle(daughters[iDaughter]);
        int PDGcode = daughters[iDaughter]->getPDGCode();

        // Is it a real photon?
        if (PDGcode == 22 && photosFlag > -0.5 && photosFlag < 0.5) {  // must not be from PHOTOS
          Status = 1.0;
        }
      }

      return Status;
    }

    VARIABLE_GROUP("DirectDaughterInfo");
    REGISTER_VARIABLE("hasCharmedDaughter(i)", hasCharmedDaughter,
                      "Returns information regarding the charm quark presence in the decay.");
    REGISTER_VARIABLE("hasCharmoniumDaughter", hasCharmoniumDaughter,
                      "Returns information regarding the charmonium state presence in the decay.");
    REGISTER_VARIABLE("hasRealPhotonDaughter", hasRealPhotonDaughter,
                      "Returns information regarding photon daughter origin for a particle.");
  }
}
