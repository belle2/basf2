/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>

#include <Math/Vector3D.h>

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

    /** Subdetector id number. */
    int m_subDet;

    /** Identifier of subdetector component. */
    int m_identifier;

    /** Track id. */
    int m_trackID;

    /** Particle position at the entrance in volume. */
    ROOT::Math::XYZVector m_startPos;

    /** Particle momentum at the entrance in volume. */
    ROOT::Math::XYZVector m_startMom;

    /** Global time. */
    double m_startTime;

    /** Particle energy at the entrance in volume. */
    double m_startEnergy;

    /** Energy deposited in volume. */
    double m_energyDeposit;

    /** Length of the track in the volume. */
    double m_trackLength;

    /** Replica (=sector) scale in ECL. */
    int m_eclrepscale;

  }; // SensitiveDetector class
} // end of namespace Belle2
