/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDIGITIZERMODULE_H
#define BKLMDIGITIZERMODULE_H

#include <TRandom3.h>

#include <framework/core/Module.h>

#include <bklm/hitbklm/BKLMSimHit.h>

#include <string>
#include <vector>
#include <queue>
#include <map>

namespace Belle2 {

  //! Convert BKLM raw simulation hits to digitizations
  class BKLMDigitizerModule : public Module {

  public:

    //! Constructor
    BKLMDigitizerModule();

    //! Destructor
    virtual ~BKLMDigitizerModule();

    //! Initialize at start of job
    virtual void initialize();

    //! Do any needed actions at the start of a simulation run
    virtual void beginRun();

    //! Digitize one event and write hits, digis, and relations into DataStore
    virtual void event();

    //! Do any needed actions at the end of a simulation run
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

  protected:

    //! Print BKLMDigitizerModule parameters
    void printModuleParameters() const;

  private:

    //! Simulation run number
    int    m_Run;

    //! Simulation event number
    int    m_Event;

  };

} // end of Belle2 namespace

#endif // BKLMDIGITIZERMODULE_H
