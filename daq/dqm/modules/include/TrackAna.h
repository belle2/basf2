/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRACKANAMODULE_H
#define TRACKANAMODULE_H

#include <framework/core/HistoModule.h>

#include "TH1F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TrackAnaModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    TrackAnaModule();
    virtual ~TrackAnaModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    //! Module funcions to define histograms
    void defineHisto() override;

    // Data members
  private:
    TH1F* h_multi;
    TH1F* h_p[4];

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
