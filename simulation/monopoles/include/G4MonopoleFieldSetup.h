/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#pragma once

#include <G4MagneticField.hh>

class G4FieldManager;
class G4ChordFinder;
class G4MagIntegratorStepper;

namespace Belle2 {

  namespace Monopoles {

    class G4MonopoleEquation;

    /**
     * Monopole field setup singleton class, that takes care of switching between conventional
     * particle transportation and monopole transportation.
     */
    class G4MonopoleFieldSetup {
    public:

      /**
       * Switches chord finder between
       * 1 - basf2 FullSim chord finder
       * 2 - monopole chord finder
       * Since monopoles use different equation of motion
       *
       * @param val Which chord finder to switch to.
       */
      void SwitchChordFinder(G4int val);

      static G4MonopoleFieldSetup* GetMonopoleFieldSetup(); /**< Returns G4MonopoleFieldSetup instance*/

      /**
       * Destructor.
       */
      ~G4MonopoleFieldSetup() ;

    private:

      /**
       * Constructor.
       */
      G4MonopoleFieldSetup();
      /**
       * Copy constructor should be hidden.
       * @param copy Copy reference.
       */
      G4MonopoleFieldSetup(const G4MonopoleFieldSetup& copy);
      /**
       * Assignment operator should be hidden.
       * @param copy Assign reference.
       */
      G4MonopoleFieldSetup& operator=(const G4MonopoleFieldSetup& copy);

      G4FieldManager*         fFieldManager ; /**< Field manager that holds current chord finder*/
      G4ChordFinder*          fMonopoleChordFinder ; /**< Chord finder for monopoles*/
      G4ChordFinder*          fbasf2ChordFinder ; /**< Chord finder taken from FullSim of basf2*/
      G4MonopoleEquation*     fMonopoleEquation ; /**< Monopole equation of motion*/
      G4MagneticField*        fMagneticField ; /**< Magnetic field of basf2*/
      G4MagIntegratorStepper* fMonopoleStepper ; /**< Stepper for monopoles*/
      G4double                fMinStep ; /**< Minimal step. 1mm is taken as default*/

      static G4MonopoleFieldSetup*  fMonopoleFieldSetup; /**< The instance of this class*/

    };

  } //end Monopoles namespace

} //end Belle2 namespace
