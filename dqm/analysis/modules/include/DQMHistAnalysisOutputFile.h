/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOut.h
// Description : DQM Analysis, dump histograms to file (as reference histograms)
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>


#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputFileModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputFileModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisOutputFileModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

    /**
     * Opens the root file and saves the content.
     */
    void save_to_file();

    // Data members
  private:

    std::string m_folder;/**< output folder to save root file */
    std::string m_prefix;/**< prefix for the output file name */
    std::string m_filename;/**< file name of root file */

    bool m_saveHistos;/**< Write all Histos to file */
    bool m_saveCanvases;/**< Write all Canvases to file */
    bool m_savePerEvent;/**< Whether save to file per event */
    bool m_savePerRun;/**< Whether save to file per run */

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;/**< event metadata object*/

  };
} // end namespace Belle2

