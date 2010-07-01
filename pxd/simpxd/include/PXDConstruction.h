/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCONSTRUCTION_H_
#define PXDCONSTRUCTION_H_

#include <simulation/simkernel/B4VSubDetectorDriver.h>

namespace Belle2 {

  //! PXD geometry construction
  /*!
      Convert PXD geometry, from TGeo to G4 geometry.
      Implement PXD sensitive detector definition, based on the prefix of volume names.
  */
  class PXDConstruction : public B4VSubDetectorDriver {

  public:

    //! Constructor
    PXDConstruction();

    //! Destructor
    virtual ~PXDConstruction() {};

    //! The method to define sensitive detector based on volume names.
    /*!
        \param dc The pointer of TG4RootDetectorConstruction to get Geant4 volumes from TGeo.
    */
    void initialize(TG4RootDetectorConstruction* dc);


  private:

  };

} // end of namespace Belle2

#endif /* PXDCONSTRUCTION_H_ */
