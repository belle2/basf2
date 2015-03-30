/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle, Jerome Baudot                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PLUME_SENSITIVEDETECTOR_H
#define PLUME_SENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PLUME detector */
  namespace plume {

    /** Sensitive Detector implementation of the PLUME detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector();
      /** Destructor*/
      ~SensitiveDetector();

    protected:
      /** Step processing method
       * @param step the G4Step with the current step information
       * @return true if a Hit has been created, false if the hit was ignored
       */
      bool step(G4Step* step, G4TouchableHistory*);
    private:
      int current_pdgID;
      int current_sensorID;
      float current_trackVertex_x;
      float current_trackVertex_y;
      float current_trackVertex_z;
      int current_trackID;
      float current_energyDep;
      float current_nielDep;
      float current_posIN_x;
      float current_posIN_y;
      float current_posIN_z;
      float current_posIN_u;
      float current_posIN_v;
      float current_posIN_w;
      float current_posOUT_x;
      float current_posOUT_y;
      float current_posOUT_z;
      float current_posOUT_u;
      float current_posOUT_v;
      float current_posOUT_w;
      float current_momentum_x;
      float current_momentum_y;
      float current_momentum_z;
      float current_thetaAngle;
      float current_phiAngle;
      float current_globalTime;

    };

  } //plume namespace
} //Belle2 namespace

#endif
