/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCCONSTRUCTION_H
#define CDCCONSTRUCTION_H

#include <simulation/simkernel/B4VSubDetectorDriver.h>

#include <string>

namespace Belle2 {

//! CDC geometry construction
  /*!
      Convert CDC - Central Drift Chamber geometry, from TGeo to G4 geometry.
      Implement CDC sensitive detector defination, based on volume names.
  */

  class CDCConstruction : public B4VSubDetectorDriver {

  public:

    //! Constructor
    CDCConstruction(void) : B4VSubDetectorDriver(std::string("CDCBelleII")) {}

    //! Destructor
    virtual ~CDCConstruction(void) {}

    //! The method to define sensitive detector based on volume names.
    /*!
        \param dc The pointer of TG4RootDetectorConstruction to get Geant4 volumes from TGeo.
    */
    void initialize(TG4RootDetectorConstruction* dc);

  private:

  }; // Class

} // end of namespace Belle2

#endif // CDCCONSTRUCTION_H 

