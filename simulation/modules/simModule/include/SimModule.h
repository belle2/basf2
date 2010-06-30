/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMMODULE_H
#define SIMMODULE_H

#include <fwcore/Module.h>
#include <boost/shared_ptr.hpp>
#include <string>

#ifdef G4VIS_USE
#include "G4VisManager.hh"
#endif

namespace Belle2 {

  class B4RunManager;

//! Module SimModule
  /*! This class is a module of Belle2 simulation.
      It inherits from basf2 module and will finish some necessary initializations in geant4,
      such as detector geometry, physics processes, particle gun generator and some user actions.
      In which, the detector geometry is a ROOT geometry gotten from gearbox directly and will be
      converted into geant4 geometry by g4root.

      In this module, users can use interactive mode and visualization function during detector simulation.
      And some parameters are also provided for changing some properties.
  */

  class SimModule : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(SimModule)

    //! Constructor
    SimModule();

    //! Destructor
    virtual ~SimModule();

    //! Initialize the Module
    /*! This method is used to initialize detector geometry, physics processes, generator, user actions and so on.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        Now, this method is used to set run id and initialize run manager.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        The method to process a event simulation.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        The method to delete pointers.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        The method to terminate current module.
    */
    virtual void terminate();

  private:

    int m_runId;
    int m_event_number;

    int m_randomSeed;
    int m_runVerbose;
    int m_eventVerbose;
    int m_trackVerbose;
    bool m_interactiveG4;
    bool m_vis;
    bool m_initialize;
    std::string m_macroName;

    B4RunManager* m_run_mgr;

#ifdef G4VIS_USE
    G4VisManager* m_visMgr;
#endif

  };

} // end namespace Belle2

#endif /* SIMMODULE_H */
