/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHSENSITIVEDETECTOR_H
#define ARICHSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <framework/database/DBObjPtr.h>
#include <arich/dbobjects/ARICHSimulationPar.h>

namespace Belle2 {
  namespace arich {
    //! The Class for ARICH Sensitive Detector
    /*! In this class, every variable defined in ARICHSimHit will be calculated,
      and stored in datastore.
    */
    class SensitiveDetector : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * Constructor.
       * @param name Name of the sensitive detector. Do we still need that?
       */
      SensitiveDetector();

      /**
       * Process each step and calculate variables defined in PXDSimHit.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);


    private:

      DBObjPtr<ARICHSimulationPar> m_simPar; /**< simulation parameters from the DB */

    }; // SensitiveDetector class
  } // end of namespace arich
} // end of namespace Belle2

#endif /* ARICHSENSITIVEDETECTOR_H */
