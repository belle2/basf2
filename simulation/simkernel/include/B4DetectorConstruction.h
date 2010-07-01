/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4DETECTORCONSTRUCTION_H
#define B4DETECTORCONSTRUCTION_H

#include <simulation/simkernel/B4VSubDetectorDriver.h>

#include "TG4RootDetectorConstruction.h"

namespace Belle2 {

  class B4MagneticField;

//! The Class for Belle2 Detector Construction
  /*! This class inherits from TVirtualUserPostDetConstruction in g4root.
      G4root is an interface allowing running native GEANT4 with a ROOT geometry.
      In fact it is just the G4 navigation that uses directly the TGeo geometry.

      The sensitive detector definition is implemented in each sub-detector.
  */

  class B4DetectorConstruction : public TVirtualUserPostDetConstruction {

  public:

    //! The B4DetectorConstruction destructor
    /*!
        Deletes the B4DetectorConstruction.
    */
    virtual ~B4DetectorConstruction();

    //! Implement the abstract method defined in base class.
    /*!
        The method to connect G4 objects to the geometry (such as: sensitive detectors, user limits, magnetic field, ...)

        \param dc The pointer of TG4RootDetectorConstruction in g4root which provides methods to acess GEANT4 created objects corresponding to TGeo ones.
    */
    virtual void Initialize(TG4RootDetectorConstruction *dc);

    //! The method to register geometry driver
    /*!
        \param aDriver A pointer of class B4VSubDetectorDriver will be stored in a vector 'm_registeredDrivers'.
    */
    void registerGeometryDriver(B4VSubDetectorDriver* aDriver);

    //! Static method to get a reference to the B4DetectorConstruction instance.
    /*!
        \return A reference to an instance of this class.
    */
    static B4DetectorConstruction* Instance();

  private:

    //! The B4DetectorConstruction constructor
    /*!
        Hidden to avoid that someone creates an instance of this class.
    */
    B4DetectorConstruction();

    std::vector<B4VSubDetectorDriver*> m_registeredDrivers; /*!< Vector to store registered detector drivers. */

    B4MagneticField* m_magneticField; /*!< Pointer that points class B4MagneticField. */

    static B4DetectorConstruction* m_instance; /*!< Pointer that saves the instance of this class. */
  };

}  // end namespace Belle2

#endif /* B4DETECTORCONSTRUCTION_H */

