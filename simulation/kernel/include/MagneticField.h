/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2017  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao, Martin Ritter                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
