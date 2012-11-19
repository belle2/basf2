/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZERMODULE_H
#define EKLMDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Module EKLMDigitizationModule.
   * @details
   * Simple module for reading EKLM hits.
   */
  class EKLMDigitizerModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMDigitizerModule();

    /**
     * Destructor
     */
    virtual ~EKLMDigitizerModule();

    /**
     * Initializer.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * This method is called for each event.
     */
    virtual void event();

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate();

  protected:
  private:

    /** Strip hits with npe lower this value will be marked as bad. */
    double m_discriminatorThreshold;

    /** Strip transformation file. */
    std::string m_stripInfromationDBFile;

  };
} // end namespace Belle2
#endif

