/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef ProgressModule_H
#define ProgressModule_H

#include <framework/core/Module.h>
#include <string>
#include <boost/format.hpp>

namespace Belle2 {

  /**
   * The Progress module.
   *
   * This module periodically writes the number of processed events/runs to the
   * logging system to give a progress indication.
   * The output is logarithmically, meaning it will output the first 10 events,
   * then every tenth event up to 100, then every hundreth event up to 1000 and
   * then every 1000th.
   *
   * The maximum interval between output can be configured.
   */
  class ProgressModule : public Module {

  public:

    /** Constructor */
    ProgressModule();

    /** Init the module */
    virtual void initialize();

    /** Show beginRun message */
    virtual void beginRun();

    /** Show progress */
    virtual void event();

  protected:
    /** Maximum order of magnitude for interval between showing progress */
    int m_maxOrder;

    /** Number of processed events. This is not the real eventNumber, just counting */
    int m_evtNr;

    /** Number of processed runs. This is not the real runNumber, just counting */
    int m_runNr;

    /** compiled output format */
    boost::format m_output;
  };
} // end namespace Belle2

#endif // ProgressModule_H
