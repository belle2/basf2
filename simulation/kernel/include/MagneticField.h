/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <G4MagneticField.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * The Class for the Belle2 magnetic field implementation for Geant4.
     *
     * The Geant4 simulation uses the central magnetic field map
     * implementation of the basf2 framework to return the magnetic field
     * vector for a space point.
     */
    class MagneticField : public G4MagneticField {

    public:

      /** Constructor. */
      MagneticField();

      /** Destructor. */
      ~MagneticField();

      /**
       * Sets the magnetic field vector for a specified space point.
       *
       * @param Point An array specifying the space point (x,y,z) in [mm].
       * @param Bfield Returns the magnetic field vector in tesla.
       */
      void GetFieldValue(const G4double Point[3], G4double* Bfield) const;
    };

  } // end namespace Simulation
} // end namespace Belle2
