/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4RUNMANAGER_H
#define B4RUNMANAGER_H

#include "globals.hh"
#include "G4RunManager.hh"

namespace Belle2 {

//! The Class for Belle2 Run Manager
  /*! This class is used in simulation module to do some initializations and processes one event simulation.
      In future, more functions will be extended.
  */

  class B4RunManager : public G4RunManager {

  public:

    //! Constructor
    B4RunManager();

    //! Destructor
    ~B4RunManager();

    //! In this method, do what you want to do at the beginning of each run.
    /*! Now, only Geant4 run initialization is implemented.
    */
    void beginRun();

    //! In this method, do what you want to do at the end of each run.
    /*! Now, only function RunTermination() is called.
    */
    void endRun();

    //! The method to process one event simulation
    /*!
        \param iEvent The current event id.
    */
    void event(int iEvent);

    //! In this method, do what you want to do at the end of each event.
    /*! Empty now.
    */
    void endOfEvent();

    //! The method to abort a run.
    /*!
        \return a bool type variable to abort current run.
    */
    bool aborted() const {
      return runAborted;
    }

    //! Static method to get a reference to the B4RunManager instance.
    /*!
        \return A reference to an instance of this class.
    */
    static B4RunManager* Instance();

  private:

    static B4RunManager* m_instance; /*!< Pointer that saves the instance of this class. */

  };

} // end namespace Belle2

#endif /* B4RUNMANAGER_H */
