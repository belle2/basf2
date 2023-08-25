/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef MONITOR_DATACOPPER_H
#define MONITOR_DATACOPPER_H

#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/HistoModule.h>

#include "TH1F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class MonitorDataCOPPERModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    MonitorDataCOPPERModule();
    virtual ~MonitorDataCOPPERModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    //! Histogram definition
    void defineHisto() override;
    virtual double getTimeSec();
    // Data members

  private:

    //! No. of sent events
    int m_loop;

    TH1* h_size;
    TH1* h_nevt;
    TH1* h_rate;
    TH1* h_diff;

    int m_nevt;

    timeval m_tv;
    double m_start_time;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
