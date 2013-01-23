/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMMUONRECONSTRUCTORMODULE_H
#define EKLMMUONRECONSTRUCTORMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMMuonReconstructorModule.
   * @details
   * Module for K0L reconstruction.
   */
  class EKLMMuonReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMMuonReconstructorModule();

    /**
     * Destructor.
     */
    ~EKLMMuonReconstructorModule();

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

  };

}

#endif

