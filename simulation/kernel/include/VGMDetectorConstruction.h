/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VGMDETECTORCONSTRUCTION_H
#define VGMDETECTORCONSTRUCTION_H

#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;

namespace Belle2 {

  namespace Simulation {

    /**
     * The Class for VGM Geant4 Belle2 Detector Construction.
     *
     */
    class VGMDetectorConstruction : public G4VUserDetectorConstruction {

    public:

      /** The VGMDetectorConstruction constructor. */
      VGMDetectorConstruction();

      /** The VGMDetectorConstruction destructor. */
      virtual ~VGMDetectorConstruction();

      virtual G4VPhysicalVolume* Construct();

    private:

    };

  } //end namespace Simulation

} //end namespace Belle2

#endif /* VGMDETECTORCONSTRUCTION_H */
