/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/StepTiming.h>
#include <framework/utilities/Utils.h>
#include <G4SteppingManager.hh>

namespace Belle2 {
  namespace Simulation {
    void StepTiming::NewStep()
    {
      //Start timing the step
      m_startTime = Utils::getClock();
    }
    void StepTiming::StepInfo()
    {
      //Step is done, get track and volume
      fTrack = fManager->GetfTrack();
      G4LogicalVolume* volume = fTrack->GetStep()->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume();
      //calculate elapsed time
      const double time = Utils::getClock() - m_startTime;
      //call callback function
      m_callback(fTrack, volume, time);
    }

  }
} //Belle2 namespace
