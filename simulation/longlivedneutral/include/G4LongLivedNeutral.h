/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from BelleII monopole simulation

#pragma once

#include <G4ParticleDefinition.hh>
#include <globals.hh>

namespace Belle2 {

  /**
  * A class to hold long-lived neutral particle description.
  *
  * Creates a GEANT4 particle via G4ParticleDefinition
  * Intended use is to let Geant 4 propagate a neutral long lived particle with defined mass and pdg value without interaction and decay it to pre-assigned decay products at a time sampled from the lifetime of the generated MCParticle. Production and decay of long-lived neutral particles should be handled by the generator.
  */
  class G4LongLivedNeutral : public G4ParticleDefinition {
  private:

    /**
     * Destructor.
     */
    virtual ~G4LongLivedNeutral();

  public:

    /**
     * Constructor.
     *
     * @param name Name of the neutral long-lived particle in GEANT4.
     */
    explicit G4LongLivedNeutral(const G4String& name);


  private:
  };

} //end Belle2 namespace
