/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4EVENTACTION_H
#define B4EVENTACTION_H

#include "G4UserEventAction.hh"
#include "G4Event.hh"

namespace Belle2 {

  //! The Class for Belle2 Event Action
  /*! This class inherits from G4UserEventAction in GEANT4.
      In this class, you can do what you want to do at the beginning and end of each event.
      For example, at the end of each event, you can save hits, do digitization and so on.
  */

  class B4EventAction : public G4UserEventAction {

  public:

    //! Constructor
    B4EventAction();

    //! Destructor
    ~B4EventAction();

    //! Called when an event starts simulation
    /*! In this method, you can get event id, primary vertex(es) and particles associated to the vertex(es).

        /param aEvent The pointer of G4Event.
    */
    void BeginOfEventAction(const G4Event* aEvent);

    //! Called when a event ends simulation
    /*!
        /param aEvent The pointer of G4Event which can access trajectories, hits collections and/or digits collections.
    */
    void EndOfEventAction(const G4Event* aEvent);

  private:

  };

} // end namespace Belle2

#endif /* B4EVENTACTION_H */
