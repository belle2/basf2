/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2010 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Martin Ritter, Igal Jaegle, Jerome Baudot, Isabelle Ripp  *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/
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

      /** particle PDG id */
      int current_pdgID;
      /** sensor ID */
      int current_sensorID;
      /** track ID */
      int current_trackID;
      /** deposited energy in electrons */
      float current_energyDep;
      /** non ionizing deposited energy */
      float current_nielDep;
      /** incoming track position x ref G4*/
      float current_posIN_x;
      /** incoming track position y ref G4*/
      float current_posIN_y;
      /** incoming track position z ref G4*/
      float current_posIN_z;
      /** incoming track position u ref mimosa*/
      float current_posIN_u;
      /** incoming track position v ref mimosa*/
      float current_posIN_v;
      /** incoming track position w ref mimosa*/
      float current_posIN_w;
      /** outgoing track position u ref mimosa*/
      float current_posOUT_u;
      /** outgoing track position v ref mimosa*/
      float current_posOUT_v;
      /** outgoing track position w ref mimosa*/
      float current_posOUT_w;
      /** incoming track momentum x*/
      float current_posOUT_x;
      /** incoming track momentum y*/
      float current_posOUT_y;
      /** incoming track momentum z*/
      float current_posOUT_z;
      /** local theta angle (out of plane) */
      float current_thetaAngle;
      /** local phi angle (in plane) */
      float current_phiAngle;
      /** global time */
      float current_globalTime;
      /** track momentum x*/
      float current_momentum_x;
      /** track momentum y*/
      float current_momentum_y;
      /** track momentum z*/
      float current_momentum_z;

    };

  } //plume namespace
} //Belle2 namespace

#endif
