/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHSENSITIVEDETECTOR_H
#define ARICHSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
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
      bool step(G4Step* aStep, G4TouchableHistory*) override;


    private:

      DBObjPtr<ARICHSimulationPar> m_simPar; /**< simulation parameters from the DB */

    }; // SensitiveDetector class
  } // end of namespace arich
} // end of namespace Belle2

#endif /* ARICHSENSITIVEDETECTOR_H */
