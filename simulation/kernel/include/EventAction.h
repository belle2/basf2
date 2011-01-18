/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTACTION_H_
#define EVENTACTION_H_

#include <G4UserEventAction.hh>
#include <G4Event.hh>

namespace Belle2 {

  namespace Simulation {

    //! The Event Action class.
    /*! This class is invoked by G4EventManager for each event.
    */
    class EventAction : public G4UserEventAction {

    public:

      //! Constructor
      EventAction();

      //! Destructor
      virtual ~EventAction();

      //! This method is invoked before converting the primary particles to G4Track objects.
      /*!
          /param event Pointer to the current G4Event object.
      */
      void BeginOfEventAction(const G4Event* event);

      //! This method is invoked at the very end of event processing.
      /*!
          /param event The pointer to the G4Event object which allows to access trajectories, hits collections and/or digits collections.
      */
      void EndOfEventAction(const G4Event* event);

    private:

    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* EVENTACTION_H_ */
