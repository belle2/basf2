/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <daq/hbasf2/utils/HLTStreamHelper.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>

#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <memory>
#include <chrono>
#include <zmq.hpp>

namespace Belle2 {
  /**
   * Module to collect DQM histograms (written out by HistoModules) and
   * send them every time period to a running ZMQ DQM server
   * (either a finalhistoserver or a proxyhistorver).
   * The address as well as the send interval are module parameters.
   * As the old DQM module, this module works by streaming everything in the current ROOT main
   * directory, which is either a TDirectory or a TH1. For the specific implementation on how
   * the streaming is done, please see the HLTStreamHelper class.
   * The histogram sending is handled via a confirmed connection (output in this case),
   * so all the usual conventions for a confirmed connection apply.
   * This module does only makes sense to run on the HLT, it is not useful for local
   * file writeout.
   */
  class HLTDQM2ZMQModule : public Module {
  public:
    /// Register the module parameters
    HLTDQM2ZMQModule();

    /**
     * On the first event, initialize the connection and the streamer.
     * If the write out interval time is reached, serialize all defined histograms and
     * send them out via a confirmed connection to the histogram server.
     * Please note, that the beginRun function calls the "defineHisto" function of
     * all histogram modules, so it needs to be called before the first event
     * (which is assured by the framework).
     */
    void event() override;

    /// Stream the histograms one last time and send out a run end message. We rely on all histogram modules to clear their own state.
    void endRun() override;

    /// Call the defineHisto function of all histogram modules registered at the RbTupleManager singleton.
    void beginRun() override;

    /// Stream the histograms one last time and send out a terminate message. We rely on all histogram modules to clear their own state.
    void terminate() override;

  private:
    /// Module parameter: histogram server address
    std::string m_param_output;
    /// Module parameter: send out interval in seconds
    unsigned int m_param_sendOutInterval = 30;

    /// ZMQ Parent needed for the connections
    std::shared_ptr<ZMQParent> m_parent;
    /// Confirmed connection to the histogram server
    std::unique_ptr<ZMQConfirmedOutput> m_output;

    /// Streamer utility. TODO: in principle we do not need this, could be static!
    HLTStreamHelper m_streamHelper;
    /// Reference to the event meta data in the data store for sending out the run and experiment number
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /// Are we still in the first event?
    bool m_firstEvent = true;
    /// Are the histograms already defined (e.g. defineHisto is called)? TODO: should we reset this after the run end?
    bool m_histogramsDefined = false;
    /// Point in time when the current interval counting started
    std::chrono::system_clock::time_point m_start;

    /// Helper function to serialize and send out the histograms
    void sendOutHistograms();
  };
}