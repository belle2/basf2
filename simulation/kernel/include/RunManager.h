/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RUNMANAGER_H_
#define RUNMANAGER_H_

#include <G4RunManager.hh>

#include <string>
#include <map>
#include <set>

namespace Belle2 {

  namespace Simulation {

    /**
     * The run manager controls the flow of the Geant4 program and manages the event loop(s) within a run.
     *
     * The run manager is also responsible for managing initialization
     * procedures, including methods in the user initialization classes.  By
     * definition of Geant4, this class has to be a singleton.
     */
    class RunManager : public G4RunManager {

    public:

      /**
       * Static method to get a reference to the RunManager instance.
       *
       * @return A reference to an instance of this class.
       */
      static RunManager& Instance();

      /** Prepares Geant4 for a new run.
       *
       * @param runNumber The current number of the run.
       */
      void beginRun(int runNumber);

      /** Process a single event in Geant4.
       *
       * @param evtNumber The current number of the event.
       */
      void processEvent(int evtNumber);

      /** Terminates a Geant4 run. */
      void endRun();

      /** Destroys the RunManager at the end of the simulation.
       *
       * Required by Geant4.
       */
      void destroy();

      /** Initialize the Kernel */
      void Initialize();

    private:

      /** The constructor is hidden to avoid that someone creates an instance of this class. */
      RunManager();

      /** Disable/Hide the copy constructor. */
      RunManager(const RunManager&);

      /** Disable/Hide the copy assignment operator. */
      RunManager& operator=(const RunManager&);

      /** The RunManager destructor. */
      virtual ~RunManager();

      static RunManager* m_instance; /**< Pointer that saves the instance of this class. */

      /** Destroyer class to delete the instance of the RunManager class when the program terminates. */
      class SingletonDestroyer {
      public: ~SingletonDestroyer() {
          // Apr.23,2012, R.Itoh
          // Removal of Singleton Instance was temporarily turned off for parallel processing
          // Should be re-activated after proper treatment is implemented.
          //          if (RunManager::m_instance != NULL) delete RunManager::m_instance;
        }
      };

      friend class SingletonDestroyer;
    };

  } //end namespace Simulation

} //end of namespace Belle2

#endif /* RUNMANAGER_H_ */
