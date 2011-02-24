/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STACKINGACTION_H_
#define STACKINGACTION_H_

#include <globals.hh>
#include <G4UserStackingAction.hh>
#include <TRandom3.h>

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

      TRandom3  m_random;    /**< The random number generator used for rejecting Cerenkov photons.*/
      double    m_photonFraction; /**< The fraction of Cerenkov photons which will be kept and propagated. */

    };

  } // Simulation namespace

} // Belle2 namespace

#endif /* STACKINGACTION_H_ */
