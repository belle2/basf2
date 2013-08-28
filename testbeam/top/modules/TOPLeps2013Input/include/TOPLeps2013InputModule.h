/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPLEPS2013INPUTMODULE_H
#define TOPLEPS2013INPUTMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <top/geometry/TOPGeometryPar.h>
#include <testbeam/top/modules/TOPLeps2013Output/TopTree.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * Intput of LEPS 2013 test beam data from a specific root ntuple ("top tree")
   */
  class TOPLeps2013InputModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPLeps2013InputModule();

    /**
     * Destructor
     */
    virtual ~TOPLeps2013InputModule();

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

    std::string m_inputFileName;  /**< input file name */
    int m_ntofMin; /**< minimal number of TOF hits to select event */
    int m_ntofMax; /**< maximal number of TOF hits to select event */
    double m_rfTimeMin; /**< minimal rf time */
    double m_x0;   /**< track starting position in x */
    double m_y0;   /**< track starting position in y */
    double m_z0;   /**< track starting position in z */
    double m_p;    /**< track momentum magnitude */
    double m_theta;  /**< track starting polar angle in degrees */
    double m_phi;    /**< track starting azimuthal angle in degrees */
    double m_t0;   /**< offset time to add for adjusting photon TDC times */

    TFile* m_file;      /**< TFile */
    TTree* m_treeTop;   /**< TTree top */
    TTree* m_treeLeps;  /**< TTree leps */
    TOP::TopTree m_top; /**< top tree structure */
    int m_ntof;         /**< number of LEPS TOF detector hits */

    TOP::TOPGeometryPar* m_topgp; /**< geometry parameters */

    int m_numPMTchannels; /**< number of PMT channels */
    int m_numChannels;    /**< number of all channels */
    float m_tdcWidth;     /**< TDC bin width in ps */
    int m_tdcOverflow;    /**< TDC overflow value */
    float m_timeMax;      /**< maximum TDC time in ps */
    int m_numEntries;     /**< number of ntuple entries */
    int m_entryCounter;   /**< entry counter */

  };

} // Belle2 namespace

#endif
