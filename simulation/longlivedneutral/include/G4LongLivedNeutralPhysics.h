/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from BelleII monopole simulation

#pragma once

#include <G4VPhysicsConstructor.hh>
#include <globals.hh>

namespace Belle2 {

  class G4LongLivedNeutral;

  /**
   LongLivedNeutral physics Class -- to be registered in the physics list
   */
  class G4LongLivedNeutralPhysics : public G4VPhysicsConstructor {
  public:

    /**
     * Constructor.
     * @param pdg
     * @param mass
     */
    explicit G4LongLivedNeutralPhysics();

    /**
     * Destructor.
     */
    ~G4LongLivedNeutralPhysics();

    /**
     * Adds monopole and anti-monopole to GEANT4 with a pdg of +/-99666
     * and parameters taken from current class.
     */
    virtual void ConstructParticle();

    /**
    * This method will be invoked in the Construct() method.
    * each physics process will be instantiated and
    * registered to the process manager of each particle type.
    */
    virtual void ConstructProcess();

  private:

    /**
     * Assignment operator should be hidden.
     * @param right Assign reference.
     */
    G4LongLivedNeutralPhysics& operator=(const G4LongLivedNeutralPhysics& right);
    /**
     * Copy constructor should be hidden.
     * @param copy Copy reference.
     */
    G4LongLivedNeutralPhysics(const G4LongLivedNeutralPhysics& copy);

    G4LongLivedNeutral* fLLPN; /**< Pointer to the neutral long lived definition in GEANT4*/
//     G4double  fMass; /**< Mass value of the neutral long lived particle*/
//     G4int     fEncoding; /**< PDG value assigned to the neutral long lived particle*/
//     G4String fParticleName; /**< Internal name assigned to the neutral long lived particle*/
  };

} //end Belle2 namespace
