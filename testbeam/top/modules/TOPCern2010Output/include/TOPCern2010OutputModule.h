/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPCERN2010OUTPUTMODULE_H
#define TOPCERN2010OUTPUTMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>
#include <TFile.h>
#include <TTree.h>


namespace Belle2 {

  /**
   * Module to output CERN2010 test beam simulation to the root ntuple.
   * The data format is according to Release v20110331
   */
  class TOPCern2010OutputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCern2010OutputModule();

    /**
     * Destructor
     */
    virtual ~TOPCern2010OutputModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:
    std::string m_outputFileName; /**< output file name */

    TFile* m_file; /**< TFile */
    TTree* m_tana; /**< TTree */

    enum {c_NumChannels = 36};  /**< number of channels */

    double m_adc[c_NumChannels];  /**< ADC values of photon hits */
    double m_tdc[c_NumChannels];  /**< TDC values of photon hits */
    double m_data_tc[4];          /**< timing counters */
    double m_data_trig[4];        /**< triger counters */
    double m_data_veto[4];        /**< veto counters */
    int m_nhit;                   /**< number of photon hits */
    double m_x1;                  /**< tracking counter 1 (x) */
    double m_y1;                  /**< tracking counter 1 (y) */
    double m_x2;                  /**< tracking counter 2 (x) */
    double m_y2;                  /**< tracking counter 2 (y) */
    double m_x3;                  /**< tracking counter 3 (x) */
    double m_y3;                  /**< tracking counter 3 (y) */
    double m_x4;                  /**< tracking counter 4 (x) */
    double m_y4;                  /**< tracking counter 4 (y) */
    double m_theta;               /**< track angle theta */
    double m_phi;                 /**< track angle phi */
    double m_x;                   /**< track position x*/
    double m_y;                   /**< track position y*/

    TOP::TOPGeometryPar* m_topgp;  /**< geometry parameters */

  };

} // Belle2 namespace

#endif
