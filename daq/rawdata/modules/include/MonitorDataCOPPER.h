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
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Histogram definition
    virtual void defineHisto();
    virtual double getTimeSec();
    // Data members

  private:

    //! No. of sent events
    int m_loop;

    int* m_buffer;

    TH1* h_size;
    TH1* h_nevt;
    TH1* h_rate;
    TH1* h_diff;
    TH1* h_hslb_size[4];
    TH1* h_hslb_nevt;
    TH1* h_hslb_rate;

    int m_nevt;
    int m_prev_nevt;

    timeval m_tv;
    double m_start_time;
    double m_prev_time;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
