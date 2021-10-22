/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

namespace Belle2 {

  /** Tracking ExpressReco DQM. */
  class TrackingExpressRecoDQMModule : public DQMHistoModuleBase {

  public:
    /** Constructor */
    TrackingExpressRecoDQMModule();
    /** Destructor */
    ~TrackingExpressRecoDQMModule() { }

    /** Module functions */
    void initialize() override;
    /** fill of the histograms happens here */
    void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    void defineHisto() override;

  private:
    /** if True, the module produces the 1D Track Residual plot for each VXD sensor*/
    bool m_produce1Dres = true;
    /** if True, the module produces the 2D Track Residual plot for each VXD sensor*/
    bool m_produce2Dres = false;

  };
}
