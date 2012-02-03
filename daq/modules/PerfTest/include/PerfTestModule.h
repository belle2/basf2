/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PERFTESTMODULE_H
#define PERFTESTMODULE_H

#include <vector>

#include <framework/core/Module.h>

namespace Belle2 {

  class PerfTestModule : public Module {

  public:
    PerfTestModule();
    virtual ~PerfTestModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  protected:
    double timeDifference(clock_t, clock_t);


  private:
    clock_t m_start;
    std::string m_outputFileName;
  };

} // end namespace Belle2

#endif // HLTINPUTMODULE_H
