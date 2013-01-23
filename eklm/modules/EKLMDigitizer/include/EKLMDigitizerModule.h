/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZERMODULE_H
#define EKLMDIGITIZERMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/Digitizer.h>

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

  private:

    /** Strip hits with npe lower this value will be marked as bad. */
    double m_discriminatorThreshold;

    /** Transformation data. */
    struct EKLM::GeometryData m_geoDat;

    /** Digitization parameters. */
    struct EKLM::DigitizationParams m_digPar;

  };

}

#endif

