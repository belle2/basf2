/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//STL
#include <string>

//Framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

//ECL
#include <ecl/utility/ECLChannelMapper.h>

class TApplication;
class TFile;

namespace Belle2 {

  class ECLCalDigit;
  class EclFrame;
  class EclData;

  /**
   * Displays energy distribution in ECL.
   *
   * EclDisplay module implements multiple graphical views of EclDigits data in
   * the user-defined event interval.
   *
   */
  class EclDisplayModule : public Module {

  public:

    /**  */
    EclDisplayModule();

    /**
     * Module destructor. Empty because all resources are freed in
     * terminate().
     */
    virtual ~EclDisplayModule();

    /**
     * Initialize EclChannelMapper.
     */
    virtual void initialize() override;

    /**
     * Empty method.
     */
    virtual void beginRun() override;

    /**
     * Handle event. Loads events into EclData and updates EclFrame
     * if m_autoDisplay flag is set.
     */
    virtual void event() override;

    /**
     * Empty method.
     */
    virtual void endRun() override;

    /**
     * Wait till EclFrame is closed then free allocated resources.
     */
    virtual void terminate() override;

  private:
    /**
     * Initialize EclFrame.
     */
    void initFrame();

    /** Show GUI. Off by default because GUI crashes automatic tests. */
    bool m_showDisplay;
    /** Keep window open after all events have been processed. Default is false. */
    bool m_keepOpen;
    /**  If true, energy distribution in ECL is displayed. Frequency of events per crystal is displayed otherwise. */
    bool m_displayEnergy;
    // TODO: List possible display mode IDs from EclPainterFactory.
    /**  Default display mode. Can be later changed in GUI. */
    int m_displayMode;
    /**  If true, events are displayed as soon as they are loaded. */
    bool m_autoDisplay{true};
    /**  filename to initialize ECLChannelMapper. */
    std::string m_eclMapperInitFileName;

    /**  Flag to check if EclFrame is closed; */
    bool m_frame_closed{false};
    /**  Counter of added events. */
    int m_evtNum{ -1};
    /**  Root GUI to display ECL data. */
    EclFrame* m_frame{nullptr};
    /**  Application to contain EclFrame. */
    TApplication* m_app{nullptr};
    /**  Class that provides interface for quick and comprehensive analysis of
     *   large number of events. */
    EclData* m_data{nullptr};
    /**  Displayed ECL events. */
    StoreArray<ECLCalDigit> m_eclarray;
    /**  Channel mapper to show channel <-> (crate, shaper) distributions. */
    ECL::ECLChannelMapper m_mapper;
    /** Temporary file to store TTree */
    TFile* m_tempfile{nullptr};
    /** Name of temporary file */
    TString m_tempname;

  public:
    /* SLOTS */
    /**
     * This method is called when EclFrame is closed.
     */
    void handleClosedFrame();
  };
}
