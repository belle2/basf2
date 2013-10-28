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
       * Constructor.
       * @param detectorID detector ID
       */
      SensitiveScintillator(int detectorID);

      /**
       * Process each step and fill simulated hits
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);

    protected:

      int m_detectorID;       /**< detector ID */
      double m_energyDeposit; /**< temporay deposited energy */
      int m_channelID;        /**< temporary channel ID */
      int m_trackID;          /**< track id */

    };

  } // end of namespace top
} // end of namespace Belle2

#endif

