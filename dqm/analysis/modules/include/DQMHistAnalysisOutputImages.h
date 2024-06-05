/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOutputImages.h
// Description : Output module for DQM Histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>


namespace Belle2 {
  /*! Class definition for the output to image module */

  class DQMHistAnalysisOutputImagesModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputImagesModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    // Data members
  private:
    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_evtMetaDataPtr;

    /** Save untagged canvase by default */
    bool m_canvasSaveDefault{true};

    /** Output path for saving images in sub-folders */
    std::string m_outputPath;

    /** flag: save as png  file*/
    bool m_asPNG{true};
    /** flag: save as jpeg file */
    bool m_asJPEG{false};
    /** flag: save as pdf  file*/
    bool m_asPDF{false};
    /** flag: save as root file */
    bool m_asROOT{false};
    /** flag: save as json file */
    bool m_asJSON{false};
    /** use and exp/run/ prefix */
    bool m_useExpRun{false};
  };
} // end namespace Belle2

