/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef SIMULATION_KERNEL_STEPTIMING_H
#define SIMULATION_KERNEL_STEPTIMING_H

#include <G4VSteppingVerbose.hh>
#include <G4LogicalVolume.hh>
#include <G4Track.hh>

namespace Belle2 {
  namespace Simulation {
    /** Class to perform more detailed timing studies of the Geant4 simulation
     *
     * This class overrides the default G4SteppingVerbose implementation which
     * is used by Geant4 to display stepping information. It measures the time
     * for each step and calls a given callback function with a pointer to the
     * track, the volume in which the step occured and the time needed for that
     * step.
     *
     * Measuring the time in each step causes a overhead of 10%-20% so this
     * should not be used in production
     */
    class StepTiming: public G4VSteppingVerbose {
    public:
      /** Signature of the callback function */
      typedef std::function<void (G4Track*, G4LogicalVolume*, double)> callbackType;
      /** Constructor: create an instance with a given callback function. */
      explicit StepTiming(const callbackType& callback): G4VSteppingVerbose(), m_callback(callback) {}
      /** Start timing a new step */
      virtual void NewStep();
      /** Step finished, call callback function */
      virtual void StepInfo();
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void AtRestDoItInvoked() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void AlongStepDoItAllDone() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void PostStepDoItAllDone() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void AlongStepDoItOneByOne() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void PostStepDoItOneByOne() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void TrackingStarted() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void DPSLStarted() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void DPSLUserLimit() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void DPSLPostStep() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void DPSLAlongStep() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void VerboseTrack() {};
      /** ignored, empty stub only implemented because it's pure virtual in base class  */
      virtual void VerboseParticleChange() {};
    private:
      /** callback function to call */
      callbackType m_callback;
      /** start time of the step */
      double m_startTime {0};
    };

  } //Simulation namespace
} //Belle2 namespace
#endif // SIMULATION_KERNEL_STEPTIMING_H
