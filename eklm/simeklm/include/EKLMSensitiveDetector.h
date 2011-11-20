/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSENSETIVEDETECTOR_H
#define EKLMSENSETIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>


namespace Belle2 {

  /**
   * The Class for EKLM Sensitive Detector
   * In this class, every variables defined in EKLMStepHit will be calculated.
   * EKLMStepHits are saved into hits collection.
   */

  class EKLMSensitiveDetector : public Simulation::SensitiveDetectorBase  {

  public:

    /**
     * Constructor
     */
    EKLMSensitiveDetector(G4String name);

    /**
     * Destructor
     */
    ~EKLMSensitiveDetector() {};

    /**
     * Register EKLM hits collection into G4HCofThisEvent
     */
    //    void Initialize(G4HCofThisEvent *eventHC);

    /**
     * Process each step and calculate variables for EKLMStepHit
     * store EKLMStepHit
     */
    bool step(G4Step *aStep, G4TouchableHistory *history);

  private:
    /**
     *  all hits with energies less than m_ThresholdEnergyDeposit
     * will be dropped
     */
    G4double m_ThresholdEnergyDeposit;

    /**
     *  all hits with time large  than m_ThresholdHitTime
     * will be dropped
     */
    G4double m_ThresholdHitTime;

  };

} // end of namespace Belle2

#endif

