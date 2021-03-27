/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2::AWESOME {

  /**
   * The Awesome module.
   *
   * This is an almost empty module which just illustrates how to define
   * parameters and use the datastore
   */
  class AwesomeModule : public Module {

  public:

    /**
     * Default constructor.
     */
    AwesomeModule();

    /**
     * Initialize the module.
     */
    void initialize() override;

    /**
     * Called for each begin of run.
     */
    void beginRun() override;

    /**
     * Called for each event.
     */
    void event() override;

    /**
     * Called for each end of run.
     */
    void endRun() override;

    /**
     * Called on termination.
     */
    void terminate() override;

  private:

    /** Useless variable showing how to create integer parameters */
    int m_intParameter;

    /** Useless variable showing how to create double parameters */
    double m_doubleParameter;

    /** Useless variable showing how to create string parameters */
    std::string m_stringParameter;

    /** Useless variable showing how to create array parameters */
    std::vector<double> m_doubleListParameter;

  };

}
