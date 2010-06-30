/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4VSUBDETECTORDRIVER_H
#define B4VSUBDETECTORDRIVER_H

#include "TG4RootDetectorConstruction.h"
#include "G4VSensitiveDetector.hh"

#include <vector>
#include <string>

#define INSTANTIATE(x) x the##x;

namespace Belle2 {

  //! The Base Class for Belle2 Detector Driver
  /*! This is a base class for Belle2 detector driver.
      Each sub-detector construction should inherit from this class and
      implement the function Initialize().
      You can take class CDCConstruction, located at libraries/cdc/simcdc/simcdc/, as an example.
  */
  class B4VSubDetectorDriver {

  public:

    //! Constructor
    /*!
        \param driverName The name of the subdetector driver.
    */
    B4VSubDetectorDriver(const std::string driverName);

    //! Destructor
    virtual ~B4VSubDetectorDriver() {};

    //! Virtual function
    /*! This is a abstract function.
        It needs to be implemented in sub-detector construction and used to
        connect G4 objects to the geometry (such as: sensitive detectors, user limits, magnetic field, ...).
    */
    virtual void initialize(TG4RootDetectorConstruction *) = 0;

    //! The method to check driver name
    /*!
        \param driverName The driver name which needs to be checked.
        \return True if this driver is available.
    */
    bool isApplicable(const std::string& driverName) const;

    //! The method to get driver name
    /*!
        \return Driver name.
    */
    std::string getName() const { return m_driverName; }


  protected:

    //! Sets the sensitive detector and automatically the sensitive volumes.
    /*!
        The sensitive volumes are identified by having a name prefix specified in the
        geometry Creator (default is "SD_").

        \param sensitiveDetector The sensitive detector class which handles the geant4 information.
        \param dc The G4Root construction object, which manages all automatic translated TGeo to geant4 objects.
    */
    void setSensitiveDetAndVolumes(G4VSensitiveDetector* sensitiveDetector, TG4RootDetectorConstruction* dc);


  private:

    std::string m_driverName; /*!< Driver name. Has to be the same as the geometry creator name. */

  };

}  // end namespace Belle2

#endif /* B4VSUBDETECTORDRIVER_H */
