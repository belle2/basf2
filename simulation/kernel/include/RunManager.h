/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RUNMANAGER_H_
#define RUNMANAGER_H_

#include <G4RunManager.hh>
#include <G4AssemblyVolume.hh>

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

      /** Register a G4AssemblyVolume */
      inline void addAssemblyVolume(G4AssemblyVolume* vol) { m_AssemblyVolumes.push_back(vol); }

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

      /** Vector of pointers to G4AssemblyVolumes */
      std::vector<G4AssemblyVolume*> m_AssemblyVolumes;

      /** Destroyer class to delete the instance of the RunManager class when the program terminates. */
      class SingletonDestroyer {
      public: ~SingletonDestroyer()
        {
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
