/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVESCINTILLATOR_H
#define TOPSENSITIVESCINTILLATOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>


namespace Belle2 {
  namespace TOPTB {

    /**
     * sensitive volume of scintillator counter
     */
    class SensitiveScintillator : public Simulation::SensitiveDetectorBase {

    public:

      /**
       * detector type enumerators
       */
      enum EDetectorType {c_undefined, c_scintillator, c_sciFi};

      /**
       * Constructor.
       * @param detectorID detector ID
       * @param type detector type
       */
      SensitiveScintillator(int detectorID, EDetectorType type);

      /**
       * Process each step and fill simulated hits
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);

    protected:

      int m_detectorID;       /**< detector ID */
      int m_type;             /**< type of detector */
      double m_energyDeposit; /**< deposited energy */
      double m_meanTime;      /**< average time */
      double m_meanX;         /**< average x coordinate in local frame */
      double m_meanY;         /**< average y coordinate in local frame */
      int m_trackID;          /**< track id */

    };

  } // end of namespace top
} // end of namespace Belle2

#endif

