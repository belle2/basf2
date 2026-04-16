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
#include <top/modules/TOPNtuple/NtupleStructure.h>

#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * Module to write out a ntuple summarizing TOP reconstruction output.
   * Used for the validation.
   */
  class TOPNtupleModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPNtupleModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    std::string m_outputFileName; /**< output file name (root file) */

    TFile* m_file = 0;      /**< TFile */
    TTree* m_tree = 0;      /**< TTree with TOPTree structure */
    TOP::TOPTree m_top;     /**< ntuple structure */

  };

} // Belle2 namespace

