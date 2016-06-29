/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMK0LRECONSTRUCTORMODULE_H
#define KLMK0LRECONSTRUCTORMODULE_H

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module KLMK0LReconstructorModule.
   * @details
   * Module for K0L reconstruction.
   */
  class KLMK0LReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMK0LReconstructorModule();

    /**
     * Destructor.
     */
    ~KLMK0LReconstructorModule();

    /**
     * Initializer.
     */
    void initialize();

    /**
     * Called when entering a new run.
     */
    void beginRun();

    /**
     * This method is called for each event.
     */
    void event();

    /**
     * This method is called if the current run ends.
     */
    void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Clustering angle. */
    double m_ClusteringAngle;

  };

}

#endif

