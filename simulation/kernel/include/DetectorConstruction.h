/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DETECTORCONSTRUCTION_H_
#define DETECTORCONSTRUCTION_H_

#include <TG4RootDetectorConstruction.h>

namespace Belle2 {

  namespace Simulation {

    /**
     * The Class for Belle2 Detector Construction.
     *
     * This class is called after the ROOT geometry has been converted to Geant4 native volumes.
     * It connects the sensitive volumes to the sensitive detector classes
     * and calls the initializeGeant4() method of all creators.
     */
    class DetectorConstruction : public TVirtualUserPostDetConstruction {

    public:

      /** The DetectorConstruction constructor. */
      DetectorConstruction();

      /** The DetectorConstruction destructor. */
      virtual ~DetectorConstruction();

      /**
       * Called by G4Root after the ROOT geometry was converted to native Geant4 volumes.
       *
       * @param dc The pointer of TG4RootDetectorConstruction in g4root which provides methods to access GEANT4 created objects
       *           which correspond to TGeo objects.
       */
      virtual void Initialize(TG4RootDetectorConstruction *dc);

    private:

    };

  } //end namespace Simulation

} //end namespace Belle2

#endif /* DETECTORCONSTRUCTION_H_ */
