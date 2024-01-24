/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef MONITOR_DATA_H
#define MONITOR_DATA_H

#include <framework/core/HistoModule.h>

#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class MonitorDataModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    MonitorDataModule();
    virtual ~MonitorDataModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    //! Histogram definition
    void defineHisto() override;

    // Data members

  private:

    //! No. of sent events
    int m_nevt;

    //! Histograms
    TH1F* h_ncpr;
    TH1F* h_nevt;
    TH1F* h_size;
    TH2F* h_size2d;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
