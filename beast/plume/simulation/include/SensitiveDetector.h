/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      bool step(G4Step* step, G4TouchableHistory*) override;
    private:

      /** particle PDG id */
      int current_pdgID;
      /** sensor ID */
      int current_sensorID;
      /** track ID */
      int current_trackID;
      /** track production vertex x coordinates in G4 ref */
      float current_trackVertex_x;
      /** track production vertex y coordinates in G4 ref */
      float current_trackVertex_y;
      /** track production vertex z coordinates in G4 ref */
      float current_trackVertex_z;
      /** deposited energy in MeV */
      float current_energyDep;
      /** non ionizing deposited energy */
      float current_nielDep;
      /** incoming track position x in G4 ref, in mm */
      float current_posIN_x;
      /** incoming track position y in G4 ref, in mm */
      float current_posIN_y;
      /** incoming track position z in G4 ref, in mm */
      float current_posIN_z;
      /** incoming track position u in sensor ref,  in mm */
      float current_posIN_u;
      /** incoming track position v in sensor ref,  in mm */
      float current_posIN_v;
      /** incoming track position w in sensor ref,  in mm */
      float current_posIN_w;
      /** outcoming track position u in sensor ref,  in mm */
      float current_posOUT_u;
      /** outcoming track position v in sensor ref,  in mm */
      float current_posOUT_v;
      /** outcoming track position w in sensor ref,  in mm */
      float current_posOUT_w;
      /** outcoming track position x in sensor ref,  in mm */
      float current_posOUT_x;
      /** outcoming track position y in sensor ref,  in mm */
      float current_posOUT_y;
      /** outcoming track position z in sensor ref,  in mm */
      float current_posOUT_z;
      /** incoming track momentum, x coordinates in G4 ref, in GeV */
      float current_momentum_x;
      /** incoming track momentum, y coordinates in G4 ref, in GeV */
      float current_momentum_y;
      /** incoming track momentum, z coordinates in G4 ref, in GeV */
      float current_momentum_z;
      /** local (sensor ref) theta angle, out of sensor plane, in degree */
      float current_thetaAngle;
      /** local (sensor ref) phi angle, in sensor plane, in degree */
      float current_phiAngle;
      /** global time */
      float current_globalTime;

    };

  } //plume namespace
} //Belle2 namespace

#endif
