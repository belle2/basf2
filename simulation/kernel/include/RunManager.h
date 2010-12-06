/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RUNMANAGER_H_
#define RUNMANAGER_H_

#include <G4RunManager.hh>

namespace Belle2 {

  /**
   * The run manager controls the flow of the Geant4 program and manages the event loop(s) within a run.
   *
   * The run manager is also responsible for managing initialization procedures, including methods in the user initialization classes.
   * By definition of Geant4, this class has to be a singleton.
   */
  class RunManager : public G4RunManager {

  public:

    /**
     * Static method to get a reference to the RunManager instance.
     *
     * @return A reference to an instance of this class.
     */
    static RunManager& Instance();


  protected:


  private:

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    RunManager();

    /** Disable/Hide the copy constructor. */
    RunManager(const RunManager&);

    /** Disable/Hide the copy assignment operator. */
    RunManager& operator=(const RunManager&);

    /** The LogSystem destructor. */
    virtual ~RunManager();

    static RunManager* m_instance; /**< Pointer that saves the instance of this class. */

    /** Destroyer class to delete the instance of the LogSystem class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (RunManager::m_instance != NULL) delete RunManager::m_instance;
      }
    };

    friend class SingletonDestroyer;
  };

} //end of namespace Belle2

#endif /* RUNMANAGER_H_ */
