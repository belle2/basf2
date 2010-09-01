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
      But, please make sure that your codes are necessary, otherwise they may slow down the program.
      At the moment only some protections are implemented in order to remove unreasonable tracks and
      to make sure that the program can run smoothly without any crash.
  */
  class B4SteppingAction : public G4UserSteppingAction {

  public:

    //! Constructor
    B4SteppingAction();

    //! Destructor
    ~B4SteppingAction();

    //! Sets the maximum number of steps before a track is stopped and killed.
    /*!
        \param maxSteps The maximum number of steps.
    */
    void setMaxNumberSteps(unsigned int maxSteps) {m_maxNumberSteps = maxSteps; };

    //! The method will be called at each step during simulation.
    /*! Add some protections to remove unreasonable tracks.
        \param aStep The pointer of current step.
    */
    void UserSteppingAction(const G4Step* aStep);


  private:

    double m_worldBoxSize[3];      /*!< The size of the simulation world box. */
    unsigned int m_maxNumberSteps; /*!< The maximum number of steps before the tracking is stopped and killed. */

  };

} // end namespace Belle2

#endif /* B4STEPPINGACTION_H */
