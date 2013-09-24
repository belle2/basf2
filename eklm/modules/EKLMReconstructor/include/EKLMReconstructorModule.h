/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECONSTRUCTORMODULE_H
#define EKLMRECONSTRUCTORMODULE_H

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMReconstructorModule.
   * @details
   * Simple module for reading EKLM hits.
   */
  class EKLMReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMReconstructorModule();

    /**
     * Destructor.
     */
    virtual ~EKLMReconstructorModule();

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

    /** Geometry data. */
    EKLM::GeometryData m_geoDat;

  };

}

#endif

