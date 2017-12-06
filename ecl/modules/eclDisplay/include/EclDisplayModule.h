/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDISPLAYMODULE_H
#define ECLDISPLAYMODULE_H

#include <rawdata/dataobjects/RawECL.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/dataobjects/ECLCalDigit.h>

#include <TSystem.h>
#include <TApplication.h>
#include <string>

#include <ecl/modules/eclDisplay/EclFrame.h>
#include <ecl/modules/eclDisplay/EclData.h>

namespace Belle2 {
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
    virtual void initialize();

    /**
     * Empty method.
     */
    virtual void beginRun();

    /**
     * Handle event. Loads events into EclData and updates EclFrame
     * if m_autoDisplay flag is set.
     */
    virtual void event();

    /**
     * Empty method.
     */
    virtual void endRun();

    /**
     * Wait till EclFrame is closed then free allocated resources.
     */
    virtual void terminate();

  private:
    /**
     * Initialize EclFrame.
     */
    void initFrame();

    /**  If true, energy distribution in ECL is displayed. Frequency of events per crystal is displayed otherwise. */
    bool m_displayEnergy;
    // TODO: List possible display mode IDs from EclPainterFactory.
    /**  Default display mode. Can be later changed in GUI. */
    int m_displayMode;
    /**  If true, events are displayed as soon as they are loaded. */
    bool m_autoDisplay;
    /**  filename to initialize ECLChannelMapper. */
    std::string m_eclMapperInitFileName;

    /**  Flag to check if EclFrame is closed; */
    bool m_frame_closed;
    /**  Counter of added events. */
    int m_evtNum;
    /**  Root GUI to display ECL data. */
    EclFrame* m_frame;
    /**  Application to contain EclFrame. */
    TApplication* m_app;
    /**  Class that provides interface for quick and comprehensive analysis of
     *   large number of events. */
    EclData* m_data;
    /**  Displayed ECL events. */
    StoreArray<ECLCalDigit> eclarray;
    /**  Channel mapper to show channel <-> (crate, shaper) distributions. */
    ECLChannelMapper m_mapper;

  public:
    /* SLOTS */
    /**
     * This method is called when EclFrame is closed.
     */
    void handleClosedFrame();

    ClassDef(EclDisplayModule, 0)
  };
}

#endif /* ECLDISPLAYMODULE_H */
