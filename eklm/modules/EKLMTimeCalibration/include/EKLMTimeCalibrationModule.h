/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATIONMODULE_H
#define EKLMTIMECALIBRATIONMODULE_H

/* C++ headers. */
#include <string>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMTimeCalibrationModule.
   * @details
   * Module for generation of transformation and alignment data.
   */
  class EKLMTimeCalibrationModule : public Module {

    /**
     * Event (hit): time, distance from hit to SiPM.
     */
    struct event {
      float time;   /**< Time. */
      float dist;   /**< Distance. */
    };

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibrationModule();

    /**
     * Destructor.
     */
    ~EKLMTimeCalibrationModule();

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

    /** Output file name. */
    std::string m_outputFileName;

    /** Output file */
    TFile* m_outputFile;

    /** Geometry data. */
    EKLM::GeometryData m_geoDat;

    /** Number of strips with different lengths. */
    int m_nStripDifferent;

    /** Trees. */
    TTree** m_Tree;

    /** Event (for tree branches). */
    struct event m_ev;

  };

}

#endif

