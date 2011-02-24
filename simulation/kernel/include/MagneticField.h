/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MAGNETICFIELD_H_
#define MAGNETICFIELD_H_

#include <geometry/bfieldmap/BFieldMap.h>

#include <G4MagneticField.hh>


namespace Belle2 {

  namespace Simulation {

    //! The Class for the Belle2 magnetic field implementation for Geant4.
    /*!
     * The field is currently a 1.5 Tesla constant magnetic field.
    */
    class MagneticField : public G4MagneticField {

    public:

      //! Constructor
      MagneticField();

      //! Destructor
      ~MagneticField();

      void GetFieldValue(const G4double Point[3], G4double *Bfield) const;

    private:

      BFieldMap& m_bField; /**< Reference to the central magnetic field map.*/

    };

  } // end namespace Simulation
} // end namespace Belle2

#endif /* MAGNETICFIELD_H_ */
