/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4STEPPINGACTION_H
#define B4STEPPINGACTION_H

#include <globals.hh>
#include <G4UserSteppingAction.hh>

namespace Belle2 {

//! The Class for Stepping Action
  /*! In principle, you can do what you want to associated to each step.
      But, please make sure that your codes are necessary, otherwise they maybe slow down the program.
      Now, I just add some protections to remove some unreasonable tracks and make sure that the program
      can run smoothly with no crash.
  */

  class B4SteppingAction : public G4UserSteppingAction {

  public:

    //! Constructor
    B4SteppingAction();

    //! Destructor
    ~B4SteppingAction();

    //! The method will be called at each step during simulation.
    /*! Add some protections to remove unreasonable tracks.
        \param aStep The pointer of current step.
    */
    void UserSteppingAction(const G4Step* aStep);

    //! If the step number of current track is greater than MaxStep, this track will be stopped and killed.
    enum { MaxStep = 100000 };

  };

} // end namespace Belle2

#endif /* B4STEPPINGACTION_H */
