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
#include <analysis/VariableManager/GeneratorSkimVariables.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Variables.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

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

      // Check if the particle has daughters
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return -999.0;
      }

      // Get the PDG sign and load daughters
      int motherPDGSign = (particle->getPDGCode()) / (abs(particle->getPDGCode()));
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = daughters[iDaughter]->getPDGCode();
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

      // Check if the particle has daughters
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return -999.0;
      }

      // Load daughters
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = daughters[iDaughter]->getPDGCode();
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

    double hasNDaughtersWithPDG(const Particle* particle, const std::vector<double>& args3)
    {
      double Status = 0.0;

      // Define what the arguments stand for
      int p_PDG = args3[0];
      int p_N = args3[1];
      int p_Sign = args3[2];

      // Check if enough arguments
      if (args3.size() != 3) {
        B2ERROR("This function accepts three arguments!");
        return -999.0;
      }

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

      // Check first argument
      if (p_PDG == 0) {
        B2ERROR("Looking for particles with PDG = 0!");
        return -999.0;
      }

      // Check second argument
      if (p_N < 0) {
        B2ERROR("Second argument should be >= 0");
        return -999.0;
      }

      // Check third argument
      if (abs(p_Sign) != 1) {
        B2ERROR("Third argument should be 1 or -1!");
        return -999.0;
      }

      // Set counter
      int Count = 0;

      // Load daughters
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = daughters[iDaughter]->getPDGCode();

        if (p_Sign == 1) {
          if (abs(daughterPDG) == abs(p_PDG)) // Same PDG codes, sign independent
            Count++;
        } else if (p_Sign == -1) {
          if (daughterPDG == p_PDG) // Same PDG codes, sign dependent
            Count++;
        }
      }

      // Check conditions
      if (Count == p_N)
        Status = 1.0;

      return Status;
    }

    double hasDaughterWithPDG(const Particle* particle, const std::vector<double>& args2)
    {
      double Status = 0.0;

      // Define what the arguments stand for
      int p_PDG = args2[0];
      int p_Sign = args2[1];

      // Check if enough arguments
      if (args2.size() != 2) {
        B2ERROR("This function accepts two arguments!");
        return -999.0;
      }

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

      // Check first argument
      if (p_PDG == 0) {
        B2ERROR("Looking for particles with PDG = 0!");
        return -999.0;
      }

      // Check second argument
      if (abs(p_Sign) != 1) {
        B2ERROR("Second argument should be 1 or -1!");
        return -999.0;
      }

      // Load daughters
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = daughters[iDaughter]->getPDGCode();

        if (p_Sign == 1) {
          if (abs(daughterPDG) == abs(p_PDG)) // same PDG codes, sign independent
            Status = 1.0;
        } else if (p_Sign == -1) {
          if (daughterPDG == p_PDG) // same PDG codes, sign dependent
            Status = 1.0;
        }
      }

      return Status;
    }

    VARIABLE_GROUP("GeneratorSkim");
    REGISTER_VARIABLE("hasCharmedDaughter(i)", hasCharmedDaughter,
                      "Returns information regarding the charm quark presence in the decay.");
    REGISTER_VARIABLE("hasCharmoniumDaughter", hasCharmoniumDaughter,
                      "Returns information regarding the charmonium state presence in the decay.");
    REGISTER_VARIABLE("hasRealPhotonDaughter", hasRealPhotonDaughter,
                      "Returns information regarding photon daughter origin for a particle.");
    REGISTER_VARIABLE("hasNDaughtersWithPDG(PDG,N,Sign)", hasNDaughtersWithPDG,
                      "Returns information regarding a specific number of daughters with a specific PDG code for a particle.");
    REGISTER_VARIABLE("hasDaughterWithPDG(PDG,Sign)", hasDaughterWithPDG,
                      "Returns information regarding any number of daughters with a specific PDG code for a particle.");

  }
}
