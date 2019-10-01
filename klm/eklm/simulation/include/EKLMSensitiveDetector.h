/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <klm/eklm/dataobjects/EKLMSimHit.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * The Class for EKLM Sensitive Detector.
     * @details
     * In this class, every variables defined in EKLMSimHit will be calculated.
     * EKLMSimHits are saved into hits collection.
     */

    class EKLMSensitiveDetector : public Simulation::SensitiveDetectorBase  {

    public:

      /**
       * Constructor.
       */
      explicit EKLMSensitiveDetector(G4String name);

      /**
       * Destructor.
       */
      ~EKLMSensitiveDetector();

      /**
       * Process each step and calculate variables for EKLMSimHit
       * store EKLMSimHit.
       */
      bool step(G4Step* aStep, G4TouchableHistory* history) override;

    private:

      /** Geometry data. */
      const EKLM::GeometryData* m_GeoDat;

      /** Simulation hits. */
      StoreArray<EKLMSimHit> m_SimHits;

      /**
       * All hits with time large  than m_ThresholdHitTime
       * will be dropped.
       */
      G4double m_ThresholdHitTime;

    };

  }

}
