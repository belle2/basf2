/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <reconstruction/modules/PIDNtuple/PIDTree.h>
#include "TFile.h"

namespace Belle2 {

  /**
   * makes PID ntuple from mdst input
   */
  class PIDNtupleModule : public Module {

  public:

    /**
     * Constructor
     */
    PIDNtupleModule();

    /**
     * Destructor
     */
    virtual ~PIDNtupleModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::string m_outputFileName; /**< output file name */
    bool m_makeFlat; /**< if true, make momentum distribution flat up to m_pMax */
    double m_p1; /**< parameter of momentum distribution */
    double m_p2; /**< parameter of momentum distribution */
    double m_pMax; /**< flatten distribution up to this momentum */

    double m_norm;  /**< distribution normalization */
    double m_value; /**< distribution value at m_pMax */

    TFile* m_file;      /**< TFile */
    TTree* m_tree;      /**< TTree with PIDTree structure */
    PID::PIDTree m_pid;      /**< PID tree structure */


    /**
     * parameterized momentum distribution
     * @param p momentum magnitude
     * @return distribution
     */
    double momDistribution(double p) const
    {
      double f = 1.0 - exp(-p / m_p1);
      return m_norm * f * f  * f * f * exp(-p / m_p2);
    };

  };

} // Belle2 namespace
