/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef STACKINGACTION_H_
#define STACKINGACTION_H_

#include <globals.hh>
#include <G4UserStackingAction.hh>

namespace Belle2 {

  namespace Simulation {

    /**
     * The basf2 stacking action.
     */
    class StackingAction : public G4UserStackingAction {

    public:

      /** The StackingAction constructor. */
      StackingAction();

      /** The StackingAction destructor. */
      ~StackingAction();

      /** Function that classifies new tracks. */
      virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack);

      /** Function that is called at each event when "urgent" stack is empty. */
      virtual void NewStage();

      /** Function called at begining of event. */
      virtual void PrepareNewEvent();

      /** Set fraction of Cerenkov photons that are actually propagated. */
      void setPropagatedPhotonFraction(double fraction) { m_photonFraction = fraction; };


    private:

      double    m_photonFraction; /**< The fraction of Cerenkov photons which will be kept and propagated. */

    };

  } // Simulation namespace

} // Belle2 namespace

#endif /* STACKINGACTION_H_ */
