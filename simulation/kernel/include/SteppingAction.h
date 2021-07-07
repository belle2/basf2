/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef STEPPINGACTION_H_
#define STEPPINGACTION_H_

#include <globals.hh>
#include <G4UserSteppingAction.hh>

#include <vector>

namespace Belle2 {

  namespace Simulation {

    /** The Class for the stepping action.
     * This class is called for each step during the Geant4 transportation.
     * It implements protection mechanisms to remove unreasonable tracks.
     */
    class SteppingAction : public G4UserSteppingAction {

    public:

      /**
       * Constructor.
       */
      SteppingAction();

      /**
       * Destructor.
       */
      virtual ~SteppingAction();

      /** Sets the maximum number of steps before a track is stopped and killed.
       * @param maxSteps The maximum number of steps.
       */
      void setMaxNumberSteps(int maxSteps) { m_maxNumberSteps = maxSteps; };

      /** Sets the trajectory option to enable storing of the simulated particle trajectories */
      void setStoreTrajectories(bool store) { m_storeTrajectories = store; }

      /** Sets the radii of absorbers for killing tracks across them.
       * This set is used in the PXD only simulation for PXD gain calibration.
       * @param vec The c++ vector with the radii of absorbers in cm
       */
      void setAbsorbersR(const std::vector<float>& vec) { m_absorbers = vec; };

      /** The method will be called at each step during simulation.
       * @param step The pointer of current step.
       */
      virtual void UserSteppingAction(const G4Step* step);


    protected:

      int m_maxNumberSteps; /**< The maximum number of steps before the track transportation is stopped and the track is killed. */
      /** if true, check if the track has attached trajectory info and append step information if necessary */
      bool m_storeTrajectories = false;
      std::vector<float> m_absorbers; /**< The absorbers defined at given radii where tracks across them will be destroyed. */


    };

  } // end namespace Simulation

} // end namespace Belle2

#endif /* STEPPINGACTION_H_ */
