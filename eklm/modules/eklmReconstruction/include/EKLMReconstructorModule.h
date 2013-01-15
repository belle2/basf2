/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECONSTRUCTIONMODULE_H
#define EKLMRECONSTRUCTIONMODULE_H

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

  protected:
  private:

    /** Transformation data. */
    struct EKLM::GeometryData m_geoDat;

    /** Light speed for first photon. */
    double m_firstPhotonlightSpeed;

    /** Time error. */
    double m_sigmaT;

  };

}

#endif

