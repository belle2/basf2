/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BACKSENSITIVEDETECTOR_H
#define BACKSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {

  //! The Class for BeamBackground Sensitive Detector
  /*! In this class, every variable defined in BeamBackHit will be calculated,
    and stored in datastore.
  */
  class BkgSensitiveDetector : public Simulation::SensitiveDetectorBase {

  public:

    /**
     * Constructor.
     * @param subDet name of the subdetector (i.e. "IR","PXD","SVD",...)
     * @param iden identifier of subdetector component (optional, 0 by default)
     */

    explicit BkgSensitiveDetector(const char* subDet, int iden = 0);

    /**
     * Process each step and calculate variables defined in PXDSimHit.
     * @param aStep Current Geant4 step in the sensitive medium.
     * @result true if a hit was stored, o.w. false.
     */
    bool step(G4Step* aStep, G4TouchableHistory*) override;


  private:
    int m_subDet;           /**< subdetector id number */
    int m_identifier;       /**< identifier of subdetector component */
    int m_trackID;          /**< track id */
    TVector3 m_startPos;    /**< particle position at the entrance in volume */
    TVector3 m_startMom;    /**< particle momentum at the entrance in volume */
    double m_startTime;     /**< global time */
    double m_startEnergy;   /**< particle energy at the entrance in volume */
    double m_energyDeposit; /**< energy deposited in volume */
    double m_trackLength;   /**< length of the track in the volume */
    int m_eclrepscale;      /**< replica (=sector) scale in ECL */
  }; // SensitiveDetector class
} // end of namespace Belle2

#endif /* BACKENSITIVEDETECTOR_H */
