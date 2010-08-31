/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4DETECTORCONSTRUCTION_H
#define B4DETECTORCONSTRUCTION_H

#include "TG4RootDetectorConstruction.h"

namespace Belle2 {

  class B4MagneticField;

  //! The Class for Belle2 Detector Construction
  /*!
      This class is called after the ROOT geometry has been
      converted to Geant4 native volumes.
      It connects the sensitive volumes to the sensitive detector classes
      and calls the initializeGeant4() method of all creators.
  */
  class B4DetectorConstruction : public TVirtualUserPostDetConstruction {

  public:

    //! The B4DetectorConstruction constructor
    B4DetectorConstruction();

    //! The B4DetectorConstruction destructor
    virtual ~B4DetectorConstruction();

    //! Called by G4Root after the ROOT geometry was converted to native Geant4 volumes.
    /*!
        \param dc The pointer of TG4RootDetectorConstruction in g4root which provides methods to access GEANT4 created objects corresponding to TGeo ones.
    */
    virtual void Initialize(TG4RootDetectorConstruction *dc);


  private:

    B4MagneticField* m_magneticField; /*!< Pointer that points class B4MagneticField. */
  };

}  // end namespace Belle2

#endif /* B4DETECTORCONSTRUCTION_H */

