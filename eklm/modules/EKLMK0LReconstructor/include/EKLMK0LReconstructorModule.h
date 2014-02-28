/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMK0LRECONSTRUCTORMODULE_H
#define EKLMK0LRECONSTRUCTORMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMK0LReconstructorModule.
   * @details
   * Module for K0L reconstruction.
   */
  class EKLMK0LReconstructorModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMK0LReconstructorModule();

    /**
     * Destructor.
     */
    ~EKLMK0LReconstructorModule();

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

    /*
    bool hasAssociatedTrack(genfit::Track &gfTrack);
    */
    /**
     * Copy information from K0Lhits array to mdst K0L
     */
    void fillMdstDataobjects();



  };

}

#endif

