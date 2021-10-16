/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>

namespace Belle2 {

  /** Tracking ExpressReco DQM. */
  class TrackingERDQMModule : public DQMHistoModuleBase {

  public:
    /** Constructor */
    TrackingERDQMModule();
    /* Destructor */
    ~TrackingERDQMModule() { }

    /** Module functions */
    virtual void initialize() override;
    virtual void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    virtual void defineHisto() override;

  protected:
  };
}
